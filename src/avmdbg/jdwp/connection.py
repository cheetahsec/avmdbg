"""JDWP transport: socket, handshake, reply correlation and event queue."""

import queue
import socket
import struct
import threading

from .constants import HANDSHAKE, HEADER_LEN, REPLY_FLAG
from .errors import JdwpConnectionError, JdwpError, JdwpTimeoutError
from .packet import Packet


class JdwpConnection:
    """A live JDWP link to a target VM (usually via ``adb forward``).

    Incoming reply packets are correlated to pending commands by packet id;
    incoming command packets (VM event notifications) are placed on a queue
    consumed with :meth:`recv_event`.
    """

    def __init__(self, host: str = "127.0.0.1", port: int = 8819, timeout: float = 10.0):
        self._timeout = timeout
        self._sock = None
        self._pending = {}
        self._pending_lock = threading.Lock()
        self._events = queue.Queue()
        self._closed = threading.Event()
        self._reader = None

        try:
            self._sock = socket.create_connection((host, port), timeout=timeout)
            self._sock.settimeout(None)
        except OSError as exc:
            raise JdwpConnectionError(f"cannot connect to {host}:{port}: {exc}") from exc

        self._handshake()
        self._reader = threading.Thread(
            target=self._reader_loop, name="avmdbg-jdwp-reader", daemon=True
        )
        self._reader.start()

    def _handshake(self) -> None:
        try:
            self._sock.sendall(HANDSHAKE)
            reply = self._recv_exact(len(HANDSHAKE))
        except OSError as exc:
            self._sock.close()
            raise JdwpConnectionError(f"JDWP handshake failed: {exc}") from exc
        if reply != HANDSHAKE:
            self._sock.close()
            raise JdwpConnectionError("JDWP handshake failed: bad handshake reply")

    def _recv_exact(self, size: int) -> bytes:
        chunks = bytearray()
        while len(chunks) < size:
            chunk = self._sock.recv(size - len(chunks))
            if not chunk:
                raise JdwpConnectionError("connection closed by target VM")
            chunks += chunk
        return bytes(chunks)

    def _reader_loop(self) -> None:
        try:
            while not self._closed.is_set():
                header = self._recv_exact(HEADER_LEN)
                (length,) = struct.unpack(">I", header[:4])
                body = self._recv_exact(length - HEADER_LEN) if length > HEADER_LEN else b""
                packet = Packet.decode(header, body)
                if packet.is_reply:
                    with self._pending_lock:
                        waiter = self._pending.get(packet.id)
                    if waiter is not None:
                        waiter.packet = packet
                        waiter.event.set()
                else:
                    self._events.put(packet)
        except (OSError, JdwpConnectionError, ValueError):
            pass
        finally:
            self._closed.set()
            with self._pending_lock:
                waiters = list(self._pending.values())
                self._pending.clear()
            for waiter in waiters:
                waiter.event.set()
            self._events.put(None)

    class _Waiter:
        def __init__(self) -> None:
            self.event = threading.Event()
            self.packet = None

    def send_command(self, cmd_set: int, cmd: int, data: bytes = b"", timeout=None) -> bytes:
        """Send a command packet and return the reply body.

        Raises :class:`JdwpError` when the VM reports a non-zero error code,
        :class:`JdwpTimeoutError` on timeout and :class:`JdwpConnectionError`
        when the link drops.
        """
        if self._closed.is_set():
            raise JdwpConnectionError("JDWP connection is closed")
        packet = Packet.command(cmd_set, cmd, data)
        waiter = self._Waiter()
        with self._pending_lock:
            self._pending[packet.id] = waiter
        try:
            self._sock.sendall(packet.encode())
        except OSError as exc:
            with self._pending_lock:
                self._pending.pop(packet.id, None)
            raise JdwpConnectionError(f"send failed: {exc}") from exc

        if not waiter.event.wait(self._timeout if timeout is None else timeout):
            with self._pending_lock:
                self._pending.pop(packet.id, None)
            raise JdwpTimeoutError(
                f"no reply for command set={cmd_set} cmd={cmd} (id={packet.id})"
            )
        if waiter.packet is None:
            raise JdwpConnectionError("JDWP connection closed while waiting for a reply")
        reply = waiter.packet
        if reply.error:
            raise JdwpError(reply.error, detail=f"command set={cmd_set} cmd={cmd}")
        return reply.data

    def recv_event(self, timeout=None) -> Packet:
        """Return the next event command packet, or ``None`` on timeout/close."""
        try:
            return self._events.get(timeout=timeout)
        except queue.Empty:
            return None

    @property
    def closed(self) -> bool:
        return self._closed.is_set()

    def close(self) -> None:
        self._closed.set()
        if self._sock is not None:
            try:
                self._sock.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            self._sock.close()
            self._sock = None
