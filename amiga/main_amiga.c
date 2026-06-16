#include "id_heads.h"
#include "rtg_driver.h"
#include <dos/dos.h>
#include <proto/exec.h>

extern longword GetTimeCount(void);
extern void InitGame(void);
extern void DemoLoop(void);
void IN_ProcessEvent(UWORD code, int pressed);

static boolean running = true;

void Quit(char *error)
{
    if (error && *error) {
        printf("Keen Error: %s\n", error);
    }
    VW_Shutdown();
    RTG_CloseScreen();
    exit(0);
}

static int handle_idcmp(struct IntuiMessage *msg)
{
    UWORD code;
    switch (msg->Class) {
    case IDCMP_RAWKEY:
        code = msg->Code;
        IN_ProcessEvent(code & ~IECODE_QUALIFIER,
            !(code & IECODE_UP_PREFIX));
        break;
    case IDCMP_VANILLAKEY:
        code = msg->Code;
        if (code == 0x1b) {
            IN_ProcessEvent(sc_Escape, 1);
        }
        break;
    case IDCMP_CLOSEWINDOW:
        Quit(NULL);
        break;
    }
    return 1;
}

static void AmigaEventHandler(void)
{
    struct IntuiMessage *msg;
    if (!rtg.window) return;
    while ((msg = (struct IntuiMessage *)GetMsg(rtg.window->UserPort))) {
        handle_idcmp(msg);
        ReplyMsg((struct Message *)msg);
    }
}

int main(int argc, char *argv[])
{
    _argc = argc;
    _argv = argv;

    if (!RTG_OpenScreen()) {
        printf("Failed to open RTG screen!\n");
        return 1;
    }

    VW_SetScreenMode(EGAGR);
    VW_SetLineWidth(SCREENWIDTH);

    bufferofs = 0;
    displayofs = 0;

    RF_SetRefreshHook(AmigaEventHandler);

    InitGame();

    DemoLoop();

    return 0;
}
