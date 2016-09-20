#!/usr/bin/python
# -*- coding: utf-8 -*-
#coding=utf-8

from avmdbg import *


class JdwpTag:
    def __init__(self):
        pass
    BYTE = 66
    CHAR = 67
    DOUBLE = 68
    FLOAT = 70
    INT = 73
    LONG = 74
    OBJECT = 76
    SHORT = 83
    BOOLEAN = 90
    ARRAY = 91


def breakPointCallback0(context):
    print "### breakPointCallback0 ###"
    # 线程、堆栈
    print "[threadId]:%d" % context["threadId"]
    print "[threadName]:%s" % context["threadName"]
    print "[frameId]:%d" % context["frameId"]

    # 输出参数
    print "[param]:"
    for k, v in context["param"].items():
        print k + ":" + str(v)

    # This指针
    thisObjectId = context["param"]["p0"]["id"]
    print "[this]:"
    objectFields = debugger.getObjectFieldValues(thisObjectId)
    for item in objectFields:
        print str(item)

    # 附加信息
    print "[reqExt]:"
    for k, v in context["reqExt"].items():
        print k + ":" + str(v)

    # 堆栈回溯
    print "[stackFrames]:"
    stackFrames = debugger.getStackFrames(context["threadId"])
    for frame in stackFrames:
        print frame["class"] + " -> " + \
              frame["method"] + frame["sign"] + " [+]" + \
              hex(frame["index"])


def breakPointCallback1(context):
    print "### breakPointCallback1 ###"
    strArray = debugger.getRegisterValue(context, "p3", JdwpTag.ARRAY)
    print "[p3]:" + str(strArray)
    print " [0]:" + str(debugger.getStringValue(strArray["value"]["data"][0]))
    print " [1]:" + str(debugger.getStringValue(strArray["value"]["data"][1]))

    print "[v0]:" + str(debugger.getRegisterValue(context, "v0", JdwpTag.OBJECT))
    print "[v1]:" + str(debugger.getRegisterValue(context, "v1", JdwpTag.INT))


if __name__ == '__main__':

    debugger = AvmDebugger()

    if not debugger.attach("com.example.x0r.demo"):
        print "[err]: process attach fail!"
        exit(-1)

    # .method private
    # test0(B, [I, J, String) V
    #     .registers 9
    #     .param p1, "a0"
    #     .param p2, "a1"
    #     .param p3, "a2"
    #     .param p5, "a3"
    #     .prologue
    bp0 = {
        "class": "Lcom/example/x0r/demo/LoginActivity;",
        "method": "test0",
        "sign": "(B[IJLjava/lang/String;)V",
        "index": 0,
        "registers": 9,
        "callback": breakPointCallback0
    }

    if not debugger.setBreakPoint(bp0):
        print "[err]: set breakpoint0 fail!"
        exit(-1)

    # @a   const-string v0, "test1"
    # @c   aget v1, p1, v2
    # @e   add-int/lit8 v1, v1, 0xb
    # @10  invoke-static {v1}, Ljava/lang/Integer;->toString(I)Ljava/lang/String;
    bp1 = {
        "class": "Lcom/example/x0r/demo/LoginActivity;",
        "method": "test1",
        "sign": "(B[IC[Ljava/lang/String;)V",
        "index": 0x10,
        "registers": 7,
        "callback": breakPointCallback1
    }

    if not debugger.setBreakPoint(bp0):
        print "[err]: set breakpoint1 fail!"
        exit(-1)

    debugger.waitLoop()
