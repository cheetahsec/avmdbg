.class public Lcom/example/x0r/demo/LoginActivity;
.super Landroid/support/v7/app/AppCompatActivity;
.source "LoginActivity.java"

# interfaces
.implements Landroid/app/LoaderManager$LoaderCallbacks;


# annotations
.annotation system Ldalvik/annotation/MemberClasses;
    value = {
        Lcom/example/x0r/demo/LoginActivity$UserLoginTask;,
        Lcom/example/x0r/demo/LoginActivity$ProfileQuery;
    }
.end annotation

.annotation system Ldalvik/annotation/Signature;
    value = {
        "Landroid/support/v7/app/AppCompatActivity;",
        "Landroid/app/LoaderManager$LoaderCallbacks",
        "<",
        "Landroid/database/Cursor;",
        ">;"
    }
.end annotation


# static fields
.field private static final DUMMY_CREDENTIALS:[Ljava/lang/String;

.field private static final REQUEST_READ_CONTACTS:I


# instance fields
.field private mAuthTask:Lcom/example/x0r/demo/LoginActivity$UserLoginTask;

.field private mEmailView:Landroid/widget/AutoCompleteTextView;

.field private mLoginFormView:Landroid/view/View;

.field private mPasswordView:Landroid/widget/EditText;

.field private mProgressView:Landroid/view/View;


# direct methods
.method static constructor <clinit>()V
    .registers 3

    #@0
    .prologue
    .line 52
    const/4 v0, 0x2

    #@1
    new-array v0, v0, [Ljava/lang/String;

    #@3
    const/4 v1, 0x0

    #@4
    const-string v2, "foo@example.com:hello"

    #@6
    aput-object v2, v0, v1

    #@8
    const/4 v1, 0x1

    #@9
    const-string v2, "bar@example.com:world"

    #@b
    aput-object v2, v0, v1

    #@d
    sput-object v0, Lcom/example/x0r/demo/LoginActivity;->DUMMY_CREDENTIALS:[Ljava/lang/String;

    #@f
    return-void
.end method

.method public constructor <init>()V
    .registers 2

    #@0
    .prologue
    .line 41
    invoke-direct {p0}, Landroid/support/v7/app/AppCompatActivity;-><init>()V

    #@3
    .line 58
    const/4 v0, 0x0

    #@4
    iput-object v0, p0, Lcom/example/x0r/demo/LoginActivity;->mAuthTask:Lcom/example/x0r/demo/LoginActivity$UserLoginTask;

    #@6
    return-void
.end method

.method static synthetic access$000(Lcom/example/x0r/demo/LoginActivity;)V
    .registers 1
    .param p0, "x0"    # Lcom/example/x0r/demo/LoginActivity;

    #@0
    .prologue
    .line 41
    invoke-direct {p0}, Lcom/example/x0r/demo/LoginActivity;->attemptLogin()V

    #@3
    return-void
.end method

.method static synthetic access$100(Lcom/example/x0r/demo/LoginActivity;B[IJLjava/lang/String;)V
    .registers 7
    .param p0, "x0"    # Lcom/example/x0r/demo/LoginActivity;
    .param p1, "x1"    # B
    .param p2, "x2"    # [I
    .param p3, "x3"    # J
    .param p5, "x4"    # Ljava/lang/String;

    #@0
    .prologue
    .line 41
    invoke-direct/range {p0 .. p5}, Lcom/example/x0r/demo/LoginActivity;->test0(B[IJLjava/lang/String;)V

    #@3
    return-void
.end method

.method static synthetic access$200(B[IC[Ljava/lang/String;)V
    .registers 4
    .param p0, "x0"    # B
    .param p1, "x1"    # [I
    .param p2, "x2"    # C
    .param p3, "x3"    # [Ljava/lang/String;

    #@0
    .prologue
    .line 41
    invoke-static {p0, p1, p2, p3}, Lcom/example/x0r/demo/LoginActivity;->test1(B[IC[Ljava/lang/String;)V

    #@3
    return-void
.end method

.method static synthetic access$300(Lcom/example/x0r/demo/LoginActivity;Ljava/lang/Integer;[S[FLandroid/view/View;)V
    .registers 5
    .param p0, "x0"    # Lcom/example/x0r/demo/LoginActivity;
    .param p1, "x1"    # Ljava/lang/Integer;
    .param p2, "x2"    # [S
    .param p3, "x3"    # [F
    .param p4, "x4"    # Landroid/view/View;

    #@0
    .prologue
    .line 41
    invoke-direct {p0, p1, p2, p3, p4}, Lcom/example/x0r/demo/LoginActivity;->test2(Ljava/lang/Integer;[S[FLandroid/view/View;)V

    #@3
    return-void
.end method

.method static synthetic access$400(Lcom/example/x0r/demo/LoginActivity;)Landroid/view/View;
    .registers 2
    .param p0, "x0"    # Lcom/example/x0r/demo/LoginActivity;

    #@0
    .prologue
    .line 41
    iget-object v0, p0, Lcom/example/x0r/demo/LoginActivity;->mLoginFormView:Landroid/view/View;

    #@2
    return-object v0
.end method

.method static synthetic access$500(Lcom/example/x0r/demo/LoginActivity;)Landroid/view/View;
    .registers 2
    .param p0, "x0"    # Lcom/example/x0r/demo/LoginActivity;

    #@0
    .prologue
    .line 41
    iget-object v0, p0, Lcom/example/x0r/demo/LoginActivity;->mProgressView:Landroid/view/View;

    #@2
    return-object v0
.end method

.method static synthetic access$600()[Ljava/lang/String;
    .registers 1

    #@0
    .prologue
    .line 41
    sget-object v0, Lcom/example/x0r/demo/LoginActivity;->DUMMY_CREDENTIALS:[Ljava/lang/String;

    #@2
    return-object v0
.end method

.method static synthetic access$702(Lcom/example/x0r/demo/LoginActivity;Lcom/example/x0r/demo/LoginActivity$UserLoginTask;)Lcom/example/x0r/demo/LoginActivity$UserLoginTask;
    .registers 2
    .param p0, "x0"    # Lcom/example/x0r/demo/LoginActivity;
    .param p1, "x1"    # Lcom/example/x0r/demo/LoginActivity$UserLoginTask;

    #@0
    .prologue
    .line 41
    iput-object p1, p0, Lcom/example/x0r/demo/LoginActivity;->mAuthTask:Lcom/example/x0r/demo/LoginActivity$UserLoginTask;

    #@2
    return-object p1
.end method

.method static synthetic access$800(Lcom/example/x0r/demo/LoginActivity;Z)V
    .registers 2
    .param p0, "x0"    # Lcom/example/x0r/demo/LoginActivity;
    .param p1, "x1"    # Z

    #@0
    .prologue
    .line 41
    invoke-direct {p0, p1}, Lcom/example/x0r/demo/LoginActivity;->showProgress(Z)V

    #@3
    return-void
.end method

.method static synthetic access$900(Lcom/example/x0r/demo/LoginActivity;)Landroid/widget/EditText;
    .registers 2
    .param p0, "x0"    # Lcom/example/x0r/demo/LoginActivity;

    #@0
    .prologue
    .line 41
    iget-object v0, p0, Lcom/example/x0r/demo/LoginActivity;->mPasswordView:Landroid/widget/EditText;

    #@2
    return-object v0
.end method

.method private addEmailsToAutoComplete(Ljava/util/List;)V
    .registers 4
    .annotation system Ldalvik/annotation/Signature;
        value = {
            "(",
            "Ljava/util/List",
            "<",
            "Ljava/lang/String;",
            ">;)V"
        }
    .end annotation

    #@0
    .prologue
    .line 314
    .local p1, "emailAddressCollection":Ljava/util/List;, "Ljava/util/List<Ljava/lang/String;>;"
    new-instance v0, Landroid/widget/ArrayAdapter;

    #@2
    const v1, 0x109000a

    #@5
    invoke-direct {v0, p0, v1, p1}, Landroid/widget/ArrayAdapter;-><init>(Landroid/content/Context;ILjava/util/List;)V

    #@8
    .line 318
    .local v0, "adapter":Landroid/widget/ArrayAdapter;, "Landroid/widget/ArrayAdapter<Ljava/lang/String;>;"
    iget-object v1, p0, Lcom/example/x0r/demo/LoginActivity;->mEmailView:Landroid/widget/AutoCompleteTextView;

    #@a
    invoke-virtual {v1, v0}, Landroid/widget/AutoCompleteTextView;->setAdapter(Landroid/widget/ListAdapter;)V

    #@d
    .line 319
    return-void
.end method

.method private attemptLogin()V
    .registers 9

    #@0
    .prologue
    const/4 v7, 0x1

    #@1
    const/4 v4, 0x0

    #@2
    .line 186
    iget-object v5, p0, Lcom/example/x0r/demo/LoginActivity;->mAuthTask:Lcom/example/x0r/demo/LoginActivity$UserLoginTask;

    #@4
    if-eqz v5, :cond_7

    #@6
    .line 230
    :goto_6
    return-void

    #@7
    .line 191
    :cond_7
    iget-object v5, p0, Lcom/example/x0r/demo/LoginActivity;->mEmailView:Landroid/widget/AutoCompleteTextView;

    #@9
    invoke-virtual {v5, v4}, Landroid/widget/AutoCompleteTextView;->setError(Ljava/lang/CharSequence;)V

    #@c
    .line 192
    iget-object v5, p0, Lcom/example/x0r/demo/LoginActivity;->mPasswordView:Landroid/widget/EditText;

    #@e
    invoke-virtual {v5, v4}, Landroid/widget/EditText;->setError(Ljava/lang/CharSequence;)V

    #@11
    .line 195
    iget-object v5, p0, Lcom/example/x0r/demo/LoginActivity;->mEmailView:Landroid/widget/AutoCompleteTextView;

    #@13
    invoke-virtual {v5}, Landroid/widget/AutoCompleteTextView;->getText()Landroid/text/Editable;

    #@16
    move-result-object v5

    #@17
    invoke-virtual {v5}, Ljava/lang/Object;->toString()Ljava/lang/String;

    #@1a
    move-result-object v1

    #@1b
    .line 196
    .local v1, "email":Ljava/lang/String;
    iget-object v5, p0, Lcom/example/x0r/demo/LoginActivity;->mPasswordView:Landroid/widget/EditText;

    #@1d
    invoke-virtual {v5}, Landroid/widget/EditText;->getText()Landroid/text/Editable;

    #@20
    move-result-object v5

    #@21
    invoke-virtual {v5}, Ljava/lang/Object;->toString()Ljava/lang/String;

    #@24
    move-result-object v3

    #@25
    .line 198
    .local v3, "password":Ljava/lang/String;
    const/4 v0, 0x0

    #@26
    .line 199
    .local v0, "cancel":Z
    const/4 v2, 0x0

    #@27
    .line 202
    .local v2, "focusView":Landroid/view/View;
    invoke-static {v3}, Landroid/text/TextUtils;->isEmpty(Ljava/lang/CharSequence;)Z

    #@2a
    move-result v5

    #@2b
    if-nez v5, :cond_42

    #@2d
    invoke-direct {p0, v3}, Lcom/example/x0r/demo/LoginActivity;->isPasswordValid(Ljava/lang/String;)Z

    #@30
    move-result v5

    #@31
    if-nez v5, :cond_42

    #@33
    .line 203
    iget-object v5, p0, Lcom/example/x0r/demo/LoginActivity;->mPasswordView:Landroid/widget/EditText;

    #@35
    const v6, 0x7f06001d

    #@38
    invoke-virtual {p0, v6}, Lcom/example/x0r/demo/LoginActivity;->getString(I)Ljava/lang/String;

    #@3b
    move-result-object v6

    #@3c
    invoke-virtual {v5, v6}, Landroid/widget/EditText;->setError(Ljava/lang/CharSequence;)V

    #@3f
    .line 204
    iget-object v2, p0, Lcom/example/x0r/demo/LoginActivity;->mPasswordView:Landroid/widget/EditText;

    #@41
    .line 205
    const/4 v0, 0x1

    #@42
    .line 209
    :cond_42
    invoke-static {v1}, Landroid/text/TextUtils;->isEmpty(Ljava/lang/CharSequence;)Z

    #@45
    move-result v5

    #@46
    if-eqz v5, :cond_5d

    #@48
    .line 210
    iget-object v5, p0, Lcom/example/x0r/demo/LoginActivity;->mEmailView:Landroid/widget/AutoCompleteTextView;

    #@4a
    const v6, 0x7f06001a

    #@4d
    invoke-virtual {p0, v6}, Lcom/example/x0r/demo/LoginActivity;->getString(I)Ljava/lang/String;

    #@50
    move-result-object v6

    #@51
    invoke-virtual {v5, v6}, Landroid/widget/AutoCompleteTextView;->setError(Ljava/lang/CharSequence;)V

    #@54
    .line 211
    iget-object v2, p0, Lcom/example/x0r/demo/LoginActivity;->mEmailView:Landroid/widget/AutoCompleteTextView;

    #@56
    .line 212
    const/4 v0, 0x1

    #@57
    .line 219
    :cond_57
    :goto_57
    if-eqz v0, :cond_73

    #@59
    .line 222
    invoke-virtual {v2}, Landroid/view/View;->requestFocus()Z

    #@5c
    goto :goto_6

    #@5d
    .line 213
    :cond_5d
    invoke-direct {p0, v1}, Lcom/example/x0r/demo/LoginActivity;->isEmailValid(Ljava/lang/String;)Z

    #@60
    move-result v5

    #@61
    if-nez v5, :cond_57

    #@63
    .line 214
    iget-object v5, p0, Lcom/example/x0r/demo/LoginActivity;->mEmailView:Landroid/widget/AutoCompleteTextView;

    #@65
    const v6, 0x7f06001c

    #@68
    invoke-virtual {p0, v6}, Lcom/example/x0r/demo/LoginActivity;->getString(I)Ljava/lang/String;

    #@6b
    move-result-object v6

    #@6c
    invoke-virtual {v5, v6}, Landroid/widget/AutoCompleteTextView;->setError(Ljava/lang/CharSequence;)V

    #@6f
    .line 215
    iget-object v2, p0, Lcom/example/x0r/demo/LoginActivity;->mEmailView:Landroid/widget/AutoCompleteTextView;

    #@71
    .line 216
    const/4 v0, 0x1

    #@72
    goto :goto_57

    #@73
    .line 226
    :cond_73
    invoke-direct {p0, v7}, Lcom/example/x0r/demo/LoginActivity;->showProgress(Z)V

    #@76
    .line 227
    new-instance v5, Lcom/example/x0r/demo/LoginActivity$UserLoginTask;

    #@78
    invoke-direct {v5, p0, v1, v3}, Lcom/example/x0r/demo/LoginActivity$UserLoginTask;-><init>(Lcom/example/x0r/demo/LoginActivity;Ljava/lang/String;Ljava/lang/String;)V

    #@7b
    iput-object v5, p0, Lcom/example/x0r/demo/LoginActivity;->mAuthTask:Lcom/example/x0r/demo/LoginActivity$UserLoginTask;

    #@7d
    .line 228
    iget-object v5, p0, Lcom/example/x0r/demo/LoginActivity;->mAuthTask:Lcom/example/x0r/demo/LoginActivity$UserLoginTask;

    #@7f
    new-array v6, v7, [Ljava/lang/Void;

    #@81
    const/4 v7, 0x0

    #@82
    check-cast v4, Ljava/lang/Void;

    #@84
    aput-object v4, v6, v7

    #@86
    invoke-virtual {v5, v6}, Lcom/example/x0r/demo/LoginActivity$UserLoginTask;->execute([Ljava/lang/Object;)Landroid/os/AsyncTask;

    #@89
    goto/16 :goto_6
.end method

.method private isEmailValid(Ljava/lang/String;)Z
    .registers 3
    .param p1, "email"    # Ljava/lang/String;

    #@0
    .prologue
    .line 234
    const-string v0, "@"

    #@2
    invoke-virtual {p1, v0}, Ljava/lang/String;->contains(Ljava/lang/CharSequence;)Z

    #@5
    move-result v0

    #@6
    return v0
.end method

.method private isPasswordValid(Ljava/lang/String;)Z
    .registers 4
    .param p1, "password"    # Ljava/lang/String;

    #@0
    .prologue
    .line 239
    invoke-virtual {p1}, Ljava/lang/String;->length()I

    #@3
    move-result v0

    #@4
    const/4 v1, 0x4

    #@5
    if-le v0, v1, :cond_9

    #@7
    const/4 v0, 0x1

    #@8
    :goto_8
    return v0

    #@9
    :cond_9
    const/4 v0, 0x0

    #@a
    goto :goto_8
.end method

.method private mayRequestContacts()Z
    .registers 5

    #@0
    .prologue
    const/4 v0, 0x1

    #@1
    const/4 v1, 0x0

    #@2
    .line 145
    sget v2, Landroid/os/Build$VERSION;->SDK_INT:I

    #@4
    const/16 v3, 0x17

    #@6
    if-ge v2, v3, :cond_9

    #@8
    .line 163
    :cond_8
    :goto_8
    return v0

    #@9
    .line 148
    :cond_9
    const-string v2, "android.permission.READ_CONTACTS"

    #@b
    invoke-virtual {p0, v2}, Lcom/example/x0r/demo/LoginActivity;->checkSelfPermission(Ljava/lang/String;)I

    #@e
    move-result v2

    #@f
    if-eqz v2, :cond_8

    #@11
    .line 151
    const-string v2, "android.permission.READ_CONTACTS"

    #@13
    invoke-virtual {p0, v2}, Lcom/example/x0r/demo/LoginActivity;->shouldShowRequestPermissionRationale(Ljava/lang/String;)Z

    #@16
    move-result v2

    #@17
    if-eqz v2, :cond_30

    #@19
    .line 152
    iget-object v0, p0, Lcom/example/x0r/demo/LoginActivity;->mEmailView:Landroid/widget/AutoCompleteTextView;

    #@1b
    const v2, 0x7f06001e

    #@1e
    const/4 v3, -0x2

    #@1f
    invoke-static {v0, v2, v3}, Landroid/support/design/widget/Snackbar;->make(Landroid/view/View;II)Landroid/support/design/widget/Snackbar;

    #@22
    move-result-object v0

    #@23
    const v2, 0x104000a

    #@26
    new-instance v3, Lcom/example/x0r/demo/LoginActivity$3;

    #@28
    invoke-direct {v3, p0}, Lcom/example/x0r/demo/LoginActivity$3;-><init>(Lcom/example/x0r/demo/LoginActivity;)V

    #@2b
    .line 153
    invoke-virtual {v0, v2, v3}, Landroid/support/design/widget/Snackbar;->setAction(ILandroid/view/View$OnClickListener;)Landroid/support/design/widget/Snackbar;

    #@2e
    :goto_2e
    move v0, v1

    #@2f
    .line 163
    goto :goto_8

    #@30
    .line 161
    :cond_30
    new-array v0, v0, [Ljava/lang/String;

    #@32
    const-string v2, "android.permission.READ_CONTACTS"

    #@34
    aput-object v2, v0, v1

    #@36
    invoke-virtual {p0, v0, v1}, Lcom/example/x0r/demo/LoginActivity;->requestPermissions([Ljava/lang/String;I)V

    #@39
    goto :goto_2e
.end method

.method private populateAutoComplete()V
    .registers 4

    #@0
    .prologue
    .line 137
    invoke-direct {p0}, Lcom/example/x0r/demo/LoginActivity;->mayRequestContacts()Z

    #@3
    move-result v0

    #@4
    if-nez v0, :cond_7

    #@6
    .line 142
    :goto_6
    return-void

    #@7
    .line 141
    :cond_7
    invoke-virtual {p0}, Lcom/example/x0r/demo/LoginActivity;->getLoaderManager()Landroid/app/LoaderManager;

    #@a
    move-result-object v0

    #@b
    const/4 v1, 0x0

    #@c
    const/4 v2, 0x0

    #@d
    invoke-virtual {v0, v1, v2, p0}, Landroid/app/LoaderManager;->initLoader(ILandroid/os/Bundle;Landroid/app/LoaderManager$LoaderCallbacks;)Landroid/content/Loader;

    #@10
    goto :goto_6
.end method

.method private showProgress(Z)V
    .registers 10
    .param p1, "show"    # Z
    .annotation build Landroid/annotation/TargetApi;
        value = 0xd
    .end annotation

    #@0
    .prologue
    const/high16 v5, 0x3f800000    # 1.0f

    #@2
    const/4 v4, 0x0

    #@3
    const/16 v2, 0x8

    #@5
    const/4 v3, 0x0

    #@6
    .line 250
    sget v1, Landroid/os/Build$VERSION;->SDK_INT:I

    #@8
    const/16 v6, 0xd

    #@a
    if-lt v1, v6, :cond_61

    #@c
    .line 251
    invoke-virtual {p0}, Lcom/example/x0r/demo/LoginActivity;->getResources()Landroid/content/res/Resources;

    #@f
    move-result-object v1

    #@10
    const/high16 v6, 0x10e0000

    #@12
    invoke-virtual {v1, v6}, Landroid/content/res/Resources;->getInteger(I)I

    #@15
    move-result v0

    #@16
    .line 253
    .local v0, "shortAnimTime":I
    iget-object v6, p0, Lcom/example/x0r/demo/LoginActivity;->mLoginFormView:Landroid/view/View;

    #@18
    if-eqz p1, :cond_59

    #@1a
    move v1, v2

    #@1b
    :goto_1b
    invoke-virtual {v6, v1}, Landroid/view/View;->setVisibility(I)V

    #@1e
    .line 254
    iget-object v1, p0, Lcom/example/x0r/demo/LoginActivity;->mLoginFormView:Landroid/view/View;

    #@20
    invoke-virtual {v1}, Landroid/view/View;->animate()Landroid/view/ViewPropertyAnimator;

    #@23
    move-result-object v1

    #@24
    int-to-long v6, v0

    #@25
    invoke-virtual {v1, v6, v7}, Landroid/view/ViewPropertyAnimator;->setDuration(J)Landroid/view/ViewPropertyAnimator;

    #@28
    move-result-object v6

    #@29
    if-eqz p1, :cond_5b

    #@2b
    move v1, v4

    #@2c
    :goto_2c
    invoke-virtual {v6, v1}, Landroid/view/ViewPropertyAnimator;->alpha(F)Landroid/view/ViewPropertyAnimator;

    #@2f
    move-result-object v1

    #@30
    new-instance v6, Lcom/example/x0r/demo/LoginActivity$4;

    #@32
    invoke-direct {v6, p0, p1}, Lcom/example/x0r/demo/LoginActivity$4;-><init>(Lcom/example/x0r/demo/LoginActivity;Z)V

    #@35
    .line 255
    invoke-virtual {v1, v6}, Landroid/view/ViewPropertyAnimator;->setListener(Landroid/animation/Animator$AnimatorListener;)Landroid/view/ViewPropertyAnimator;

    #@38
    .line 262
    iget-object v1, p0, Lcom/example/x0r/demo/LoginActivity;->mProgressView:Landroid/view/View;

    #@3a
    if-eqz p1, :cond_5d

    #@3c
    :goto_3c
    invoke-virtual {v1, v3}, Landroid/view/View;->setVisibility(I)V

    #@3f
    .line 263
    iget-object v1, p0, Lcom/example/x0r/demo/LoginActivity;->mProgressView:Landroid/view/View;

    #@41
    invoke-virtual {v1}, Landroid/view/View;->animate()Landroid/view/ViewPropertyAnimator;

    #@44
    move-result-object v1

    #@45
    int-to-long v2, v0

    #@46
    invoke-virtual {v1, v2, v3}, Landroid/view/ViewPropertyAnimator;->setDuration(J)Landroid/view/ViewPropertyAnimator;

    #@49
    move-result-object v1

    #@4a
    if-eqz p1, :cond_5f

    #@4c
    :goto_4c
    invoke-virtual {v1, v5}, Landroid/view/ViewPropertyAnimator;->alpha(F)Landroid/view/ViewPropertyAnimator;

    #@4f
    move-result-object v1

    #@50
    new-instance v2, Lcom/example/x0r/demo/LoginActivity$5;

    #@52
    invoke-direct {v2, p0, p1}, Lcom/example/x0r/demo/LoginActivity$5;-><init>(Lcom/example/x0r/demo/LoginActivity;Z)V

    #@55
    .line 264
    invoke-virtual {v1, v2}, Landroid/view/ViewPropertyAnimator;->setListener(Landroid/animation/Animator$AnimatorListener;)Landroid/view/ViewPropertyAnimator;

    #@58
    .line 276
    .end local v0    # "shortAnimTime":I
    :goto_58
    return-void

    #@59
    .restart local v0    # "shortAnimTime":I
    :cond_59
    move v1, v3

    #@5a
    .line 253
    goto :goto_1b

    #@5b
    :cond_5b
    move v1, v5

    #@5c
    .line 254
    goto :goto_2c

    #@5d
    :cond_5d
    move v3, v2

    #@5e
    .line 262
    goto :goto_3c

    #@5f
    :cond_5f
    move v5, v4

    #@60
    .line 263
    goto :goto_4c

    #@61
    .line 273
    .end local v0    # "shortAnimTime":I
    :cond_61
    iget-object v4, p0, Lcom/example/x0r/demo/LoginActivity;->mProgressView:Landroid/view/View;

    #@63
    if-eqz p1, :cond_71

    #@65
    move v1, v3

    #@66
    :goto_66
    invoke-virtual {v4, v1}, Landroid/view/View;->setVisibility(I)V

    #@69
    .line 274
    iget-object v1, p0, Lcom/example/x0r/demo/LoginActivity;->mLoginFormView:Landroid/view/View;

    #@6b
    if-eqz p1, :cond_73

    #@6d
    :goto_6d
    invoke-virtual {v1, v2}, Landroid/view/View;->setVisibility(I)V

    #@70
    goto :goto_58

    #@71
    :cond_71
    move v1, v2

    #@72
    .line 273
    goto :goto_66

    #@73
    :cond_73
    move v2, v3

    #@74
    .line 274
    goto :goto_6d
.end method

.method private test0(B[IJLjava/lang/String;)V
    .registers 9
    .param p1, "a0"    # B
    .param p2, "a1"    # [I
    .param p3, "a2"    # J
    .param p5, "a3"    # Ljava/lang/String;

    #@0
    .prologue
    .line 107
    const-string v0, "test0"

    #@2
    invoke-static {p1}, Ljava/lang/Byte;->toString(B)Ljava/lang/String;

    #@5
    move-result-object v1

    #@6
    invoke-static {v0, v1}, Landroid/util/Log;->v(Ljava/lang/String;Ljava/lang/String;)I

    #@9
    .line 108
    const-string v0, "test0"

    #@b
    const/4 v1, 0x1

    #@c
    aget v1, p2, v1

    #@e
    add-int/lit8 v1, v1, 0xa

    #@10
    invoke-static {v1}, Ljava/lang/Integer;->toString(I)Ljava/lang/String;

    #@13
    move-result-object v1

    #@14
    invoke-static {v0, v1}, Landroid/util/Log;->v(Ljava/lang/String;Ljava/lang/String;)I

    #@17
    .line 109
    const-string v0, "test0"

    #@19
    invoke-static {p3, p4}, Ljava/lang/Long;->toString(J)Ljava/lang/String;

    #@1c
    move-result-object v1

    #@1d
    invoke-static {v0, v1}, Landroid/util/Log;->v(Ljava/lang/String;Ljava/lang/String;)I

    #@20
    .line 110
    const-string v0, "test0"

    #@22
    invoke-static {v0, p5}, Landroid/util/Log;->v(Ljava/lang/String;Ljava/lang/String;)I

    #@25
    .line 111
    return-void
.end method

.method private static test1(B[IC[Ljava/lang/String;)V
    .registers 7
    .param p0, "a0"    # B
    .param p1, "a1"    # [I
    .param p2, "a2"    # C
    .param p3, "a3"    # [Ljava/lang/String;

    #@0
    .prologue
    const/4 v2, 0x0

    #@1
    .line 114
    const-string v0, "test1"

    #@3
    invoke-static {p0}, Ljava/lang/Byte;->toString(B)Ljava/lang/String;

    #@6
    move-result-object v1

    #@7
    invoke-static {v0, v1}, Landroid/util/Log;->v(Ljava/lang/String;Ljava/lang/String;)I

    #@a
    .line 115
    const-string v0, "test1"

    #@c
    aget v1, p1, v2

    #@e
    add-int/lit8 v1, v1, 0xb

    #@10
    invoke-static {v1}, Ljava/lang/Integer;->toString(I)Ljava/lang/String;

    #@13
    move-result-object v1

    #@14
    invoke-static {v0, v1}, Landroid/util/Log;->v(Ljava/lang/String;Ljava/lang/String;)I

    #@17
    .line 116
    const-string v0, "test1"

    #@19
    invoke-static {p2}, Ljava/lang/Character;->toString(C)Ljava/lang/String;

    #@1c
    move-result-object v1

    #@1d
    invoke-static {v0, v1}, Landroid/util/Log;->v(Ljava/lang/String;Ljava/lang/String;)I

    #@20
    .line 117
    const-string v0, "test1"

    #@22
    aget-object v1, p3, v2

    #@24
    invoke-static {v0, v1}, Landroid/util/Log;->v(Ljava/lang/String;Ljava/lang/String;)I

    #@27
    .line 118
    return-void
.end method

.method private test2(Ljava/lang/Integer;[S[FLandroid/view/View;)V
    .registers 9
    .param p1, "a0"    # Ljava/lang/Integer;
    .param p2, "a1"    # [S
    .param p3, "a2"    # [F
    .param p4, "view"    # Landroid/view/View;

    #@0
    .prologue
    const/4 v3, 0x2

    #@1
    .line 122
    const-string v1, "test2"

    #@3
    invoke-virtual {p1}, Ljava/lang/Integer;->toString()Ljava/lang/String;

    #@6
    move-result-object v2

    #@7
    invoke-static {v1, v2}, Landroid/util/Log;->v(Ljava/lang/String;Ljava/lang/String;)I

    #@a
    .line 124
    const/4 v0, 0x0

    #@b
    .local v0, "i":I
    :goto_b
    if-ge v0, v3, :cond_1b

    #@d
    .line 125
    const-string v1, "test2"

    #@f
    aget-short v2, p2, v0

    #@11
    invoke-static {v2}, Ljava/lang/Short;->toString(S)Ljava/lang/String;

    #@14
    move-result-object v2

    #@15
    invoke-static {v1, v2}, Landroid/util/Log;->v(Ljava/lang/String;Ljava/lang/String;)I

    #@18
    .line 124
    add-int/lit8 v0, v0, 0x1

    #@1a
    goto :goto_b

    #@1b
    .line 128
    :cond_1b
    const/4 v0, 0x0

    #@1c
    :goto_1c
    if-ge v0, v3, :cond_2c

    #@1e
    .line 129
    const-string v1, "test2"

    #@20
    aget v2, p3, v0

    #@22
    invoke-static {v2}, Ljava/lang/Float;->toString(F)Ljava/lang/String;

    #@25
    move-result-object v2

    #@26
    invoke-static {v1, v2}, Landroid/util/Log;->v(Ljava/lang/String;Ljava/lang/String;)I

    #@29
    .line 128
    add-int/lit8 v0, v0, 0x1

    #@2b
    goto :goto_1c

    #@2c
    .line 132
    :cond_2c
    const-string v1, "test2"

    #@2e
    invoke-virtual {p4}, Landroid/view/View;->toString()Ljava/lang/String;

    #@31
    move-result-object v2

    #@32
    invoke-static {v1, v2}, Landroid/util/Log;->v(Ljava/lang/String;Ljava/lang/String;)I

    #@35
    .line 133
    return-void
.end method


# virtual methods
.method protected onCreate(Landroid/os/Bundle;)V
    .registers 5
    .param p1, "savedInstanceState"    # Landroid/os/Bundle;

    #@0
    .prologue
    .line 68
    invoke-super {p0, p1}, Landroid/support/v7/app/AppCompatActivity;->onCreate(Landroid/os/Bundle;)V

    #@3
    .line 69
    const v1, 0x7f040019

    #@6
    invoke-virtual {p0, v1}, Lcom/example/x0r/demo/LoginActivity;->setContentView(I)V

    #@9
    .line 71
    const v1, 0x7f0c006c

    #@c
    invoke-virtual {p0, v1}, Lcom/example/x0r/demo/LoginActivity;->findViewById(I)Landroid/view/View;

    #@f
    move-result-object v1

    #@10
    check-cast v1, Landroid/widget/AutoCompleteTextView;

    #@12
    iput-object v1, p0, Lcom/example/x0r/demo/LoginActivity;->mEmailView:Landroid/widget/AutoCompleteTextView;

    #@14
    .line 72
    invoke-direct {p0}, Lcom/example/x0r/demo/LoginActivity;->populateAutoComplete()V

    #@17
    .line 74
    const v1, 0x7f0c006d

    #@1a
    invoke-virtual {p0, v1}, Lcom/example/x0r/demo/LoginActivity;->findViewById(I)Landroid/view/View;

    #@1d
    move-result-object v1

    #@1e
    check-cast v1, Landroid/widget/EditText;

    #@20
    iput-object v1, p0, Lcom/example/x0r/demo/LoginActivity;->mPasswordView:Landroid/widget/EditText;

    #@22
    .line 75
    iget-object v1, p0, Lcom/example/x0r/demo/LoginActivity;->mPasswordView:Landroid/widget/EditText;

    #@24
    new-instance v2, Lcom/example/x0r/demo/LoginActivity$1;

    #@26
    invoke-direct {v2, p0}, Lcom/example/x0r/demo/LoginActivity$1;-><init>(Lcom/example/x0r/demo/LoginActivity;)V

    #@29
    invoke-virtual {v1, v2}, Landroid/widget/EditText;->setOnEditorActionListener(Landroid/widget/TextView$OnEditorActionListener;)V

    #@2c
    .line 86
    const v1, 0x7f0c006f

    #@2f
    invoke-virtual {p0, v1}, Lcom/example/x0r/demo/LoginActivity;->findViewById(I)Landroid/view/View;

    #@32
    move-result-object v0

    #@33
    check-cast v0, Landroid/widget/Button;

    #@35
    .line 87
    .local v0, "mEmailSignInButton":Landroid/widget/Button;
    new-instance v1, Lcom/example/x0r/demo/LoginActivity$2;

    #@37
    invoke-direct {v1, p0}, Lcom/example/x0r/demo/LoginActivity$2;-><init>(Lcom/example/x0r/demo/LoginActivity;)V

    #@3a
    invoke-virtual {v0, v1}, Landroid/widget/Button;->setOnClickListener(Landroid/view/View$OnClickListener;)V

    #@3d
    .line 101
    const v1, 0x7f0c006a

    #@40
    invoke-virtual {p0, v1}, Lcom/example/x0r/demo/LoginActivity;->findViewById(I)Landroid/view/View;

    #@43
    move-result-object v1

    #@44
    iput-object v1, p0, Lcom/example/x0r/demo/LoginActivity;->mLoginFormView:Landroid/view/View;

    #@46
    .line 102
    const v1, 0x7f0c0069

    #@49
    invoke-virtual {p0, v1}, Lcom/example/x0r/demo/LoginActivity;->findViewById(I)Landroid/view/View;

    #@4c
    move-result-object v1

    #@4d
    iput-object v1, p0, Lcom/example/x0r/demo/LoginActivity;->mProgressView:Landroid/view/View;

    #@4f
    .line 103
    return-void
.end method

.method public onCreateLoader(ILandroid/os/Bundle;)Landroid/content/Loader;
    .registers 10
    .param p1, "i"    # I
    .param p2, "bundle"    # Landroid/os/Bundle;
    .annotation system Ldalvik/annotation/Signature;
        value = {
            "(I",
            "Landroid/os/Bundle;",
            ")",
            "Landroid/content/Loader",
            "<",
            "Landroid/database/Cursor;",
            ">;"
        }
    .end annotation

    #@0
    .prologue
    .line 280
    new-instance v0, Landroid/content/CursorLoader;

    #@2
    sget-object v1, Landroid/provider/ContactsContract$Profile;->CONTENT_URI:Landroid/net/Uri;

    #@4
    const-string v2, "data"

    #@6
    .line 282
    invoke-static {v1, v2}, Landroid/net/Uri;->withAppendedPath(Landroid/net/Uri;Ljava/lang/String;)Landroid/net/Uri;

    #@9
    move-result-object v2

    #@a
    sget-object v3, Lcom/example/x0r/demo/LoginActivity$ProfileQuery;->PROJECTION:[Ljava/lang/String;

    #@c
    const-string v4, "mimetype = ?"

    #@e
    const/4 v1, 0x1

    #@f
    new-array v5, v1, [Ljava/lang/String;

    #@11
    const/4 v1, 0x0

    #@12
    const-string v6, "vnd.android.cursor.item/email_v2"

    #@14
    aput-object v6, v5, v1

    #@16
    const-string v6, "is_primary DESC"

    #@18
    move-object v1, p0

    #@19
    invoke-direct/range {v0 .. v6}, Landroid/content/CursorLoader;-><init>(Landroid/content/Context;Landroid/net/Uri;[Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;)V

    #@1c
    return-object v0
.end method

.method public onLoadFinished(Landroid/content/Loader;Landroid/database/Cursor;)V
    .registers 5
    .param p2, "cursor"    # Landroid/database/Cursor;
    .annotation system Ldalvik/annotation/Signature;
        value = {
            "(",
            "Landroid/content/Loader",
            "<",
            "Landroid/database/Cursor;",
            ">;",
            "Landroid/database/Cursor;",
            ")V"
        }
    .end annotation

    #@0
    .prologue
    .line 297
    .local p1, "cursorLoader":Landroid/content/Loader;, "Landroid/content/Loader<Landroid/database/Cursor;>;"
    new-instance v0, Ljava/util/ArrayList;

    #@2
    invoke-direct {v0}, Ljava/util/ArrayList;-><init>()V

    #@5
    .line 298
    .local v0, "emails":Ljava/util/List;, "Ljava/util/List<Ljava/lang/String;>;"
    invoke-interface {p2}, Landroid/database/Cursor;->moveToFirst()Z

    #@8
    .line 299
    :goto_8
    invoke-interface {p2}, Landroid/database/Cursor;->isAfterLast()Z

    #@b
    move-result v1

    #@c
    if-nez v1, :cond_1a

    #@e
    .line 300
    const/4 v1, 0x0

    #@f
    invoke-interface {p2, v1}, Landroid/database/Cursor;->getString(I)Ljava/lang/String;

    #@12
    move-result-object v1

    #@13
    invoke-interface {v0, v1}, Ljava/util/List;->add(Ljava/lang/Object;)Z

    #@16
    .line 301
    invoke-interface {p2}, Landroid/database/Cursor;->moveToNext()Z

    #@19
    goto :goto_8

    #@1a
    .line 304
    :cond_1a
    invoke-direct {p0, v0}, Lcom/example/x0r/demo/LoginActivity;->addEmailsToAutoComplete(Ljava/util/List;)V

    #@1d
    .line 305
    return-void
.end method

.method public bridge synthetic onLoadFinished(Landroid/content/Loader;Ljava/lang/Object;)V
    .registers 3

    #@0
    .prologue
    .line 41
    check-cast p2, Landroid/database/Cursor;

    #@2
    invoke-virtual {p0, p1, p2}, Lcom/example/x0r/demo/LoginActivity;->onLoadFinished(Landroid/content/Loader;Landroid/database/Cursor;)V

    #@5
    return-void
.end method

.method public onLoaderReset(Landroid/content/Loader;)V
    .registers 2
    .annotation system Ldalvik/annotation/Signature;
        value = {
            "(",
            "Landroid/content/Loader",
            "<",
            "Landroid/database/Cursor;",
            ">;)V"
        }
    .end annotation

    #@0
    .prologue
    .line 310
    .local p1, "cursorLoader":Landroid/content/Loader;, "Landroid/content/Loader<Landroid/database/Cursor;>;"
    return-void
.end method

.method public onRequestPermissionsResult(I[Ljava/lang/String;[I)V
    .registers 6
    .param p1, "requestCode"    # I
    .param p2, "permissions"    # [Ljava/lang/String;
        .annotation build Landroid/support/annotation/NonNull;
        .end annotation
    .end param
    .param p3, "grantResults"    # [I
        .annotation build Landroid/support/annotation/NonNull;
        .end annotation
    .end param

    #@0
    .prologue
    .line 172
    if-nez p1, :cond_e

    #@2
    .line 173
    array-length v0, p3

    #@3
    const/4 v1, 0x1

    #@4
    if-ne v0, v1, :cond_e

    #@6
    const/4 v0, 0x0

    #@7
    aget v0, p3, v0

    #@9
    if-nez v0, :cond_e

    #@b
    .line 174
    invoke-direct {p0}, Lcom/example/x0r/demo/LoginActivity;->populateAutoComplete()V

    #@e
    .line 177
    :cond_e
    return-void
.end method
