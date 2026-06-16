#include "id_heads.h"
#include <devices/timer.h>
#include <dos/dos.h>

boolean LeaveDriveOn;
boolean SoundSourcePresent, SoundBlasterPresent, AdLibPresent;
boolean NeedsDigitized, NeedsMusic;
SDMode SoundMode;
SMMode MusicMode;
longword TimeCount;
boolean ssIsTandy;
word ssPort;

static struct MsgPort *timer_port;
static struct timerequest *timer_io;
static longword base_time;

void SD_Startup(void)
{
    TimeCount = 0;
    SoundMode = sdm_Off;
    MusicMode = smm_Off;
    NeedsDigitized = false;
    NeedsMusic = false;

    timer_port = CreateMsgPort();
    if (timer_port) {
        timer_io = (struct timerequest *)CreateIORequest(timer_port,
                        sizeof(struct timerequest));
        if (timer_io) {
            if (OpenDevice("timer.device", UNIT_MICRO,
                (struct IORequest *)timer_io, 0)) {
                DeleteIORequest((struct IORequest *)timer_io);
                timer_io = NULL;
            } else {
                timer_io->tr_node.io_Command = TRDERTASK;
                timer_io->tr_time.tv_secs = 0;
                timer_io->tr_time.tv_micro = 1000000 / TickBase;
                DoIO((struct IORequest *)timer_io);
                base_time = 0;
            }
        }
    }
}

void SD_Shutdown(void)
{
    if (timer_io) {
        CloseDevice((struct IORequest *)timer_io);
        DeleteIORequest((struct IORequest *)timer_io);
    }
    if (timer_port)
        DeleteMsgPort(timer_port);
}

void SD_Default(boolean gotit, SDMode sd, SMMode sm)
{
    SoundMode = sd;
    MusicMode = sm;
}

void SD_PlaySound(word sound) {}
void SD_StopSound(void) {}
void SD_WaitSoundDone(void) {}

void SD_StartMusic(Ptr music) {}
void SD_FadeOutMusic(void) {}

void SD_SetUserHook(void (*hook)(void)) {}

boolean SD_MusicPlaying(void) { return false; }

boolean SD_SetSoundMode(SDMode mode)
{
    SoundMode = mode;
    return true;
}

boolean SD_SetMusicMode(SMMode mode)
{
    MusicMode = mode;
    return true;
}

word SD_SoundPlaying(void) { return 0; }

longword GetTimeCount(void)
{
    if (timer_io) {
        timer_io->tr_node.io_Command = TRDERTASK;
        DoIO((struct IORequest *)timer_io);
        TimeCount = timer_io->tr_time.tv_secs * TickBase
                  + timer_io->tr_time.tv_micro / (1000000 / TickBase);
    }
    return TimeCount;
}
