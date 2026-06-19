#include "id_heads.h"
#include "id_ca.h"
#include "audiokdr.h"
#include <devices/timer.h>
#include <devices/ahi.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <string.h>
#include <stdlib.h>

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

static struct MsgPort *ahi_mp;
static struct IOAHIREQ *ahi_io;
static BOOL ahi_ok = FALSE;
static UBYTE ahi_buf[65536];

static word sound_priority;
static word sound_number;

void SD_Startup(void)
{
    TimeCount = 0;
    SoundMode = sdm_Off;
    MusicMode = smm_Off;
    NeedsDigitized = false;
    NeedsMusic = false;
    sound_number = 0;
    sound_priority = 0;

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
            }
        }
    }

    ahi_mp = CreateMsgPort();
    if (ahi_mp) {
        ahi_io = (struct IOAHIREQ *)CreateIORequest(ahi_mp,
                        sizeof(struct IOAHIREQ));
        if (ahi_io) {
            if (OpenDevice(AHINAME, 0, (struct IORequest *)ahi_io, 0)) {
                DeleteIORequest((struct IORequest *)ahi_io);
                ahi_io = NULL;
            } else {
                ahi_ok = TRUE;
            }
        }
        if (!ahi_ok) {
            DeleteMsgPort(ahi_mp);
            ahi_mp = NULL;
        }
    }
}

void SD_Shutdown(void)
{
    if (ahi_io && ahi_ok) {
        ahi_io->ior.io_Command = AHICMD_STOP;
        DoIO((struct IORequest *)ahi_io);
    }
    if (ahi_io) {
        CloseDevice((struct IORequest *)ahi_io);
        DeleteIORequest((struct IORequest *)ahi_io);
    }
    if (ahi_mp)
        DeleteMsgPort(ahi_mp);

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

void SD_PlaySound(word sound)
{
    SampledSound *s;
    ULONG len, i;
    struct IOAHIREQ *pending;

    if (SoundMode == sdm_Off)
        return;

    if (sound >= NUMSOUNDS)
        return;

    s = (SampledSound *)audiosegs[STARTDIGISOUNDS + sound];
    if (!s)
        return;
    if (s->common.priority < sound_priority)
        return;

    /* Drain any completed async playback */
    pending = (struct IOAHIREQ *)GetMsg(ahi_mp);
    if (pending)
        ReplyMsg((struct Message *)pending);

    /* Stop current sound */
    ahi_io->ior.io_Command = AHICMD_STOP;
    DoIO((struct IORequest *)ahi_io);

    /* Copy and convert unsigned 8-bit to signed 8-bit for AHI */
    len = s->common.length;
    if (len > sizeof(ahi_buf))
        len = sizeof(ahi_buf);
    for (i = 0; i < len; i++)
        ahi_buf[i] = s->data[i] ^ 0x80;

    /* Start AHI playback (async) */
    ahi_io->ahir_Version = 4;
    ahi_io->ahir_Volume = 0x10000;
    ahi_io->ahir_Position = 0x8000;
    ahi_io->ahir_Frequency = s->hertz;
    ahi_io->ahir_Type = AHIST_M8S;
    ahi_io->ahir_Address = ahi_buf;
    ahi_io->ahir_Length = len;
    ahi_io->ahir_Sound = 0;
    ahi_io->ior.io_Command = AHICMD_PLAY;
    SendIO((struct IORequest *)ahi_io);

    sound_number = sound;
    sound_priority = s->common.priority;
}

void SD_StopSound(void)
{
    struct IOAHIREQ *pending;

    if (ahi_io && ahi_ok) {
        ahi_io->ior.io_Command = AHICMD_STOP;
        DoIO((struct IORequest *)ahi_io);

        /* Drain any pending completion */
        pending = (struct IOAHIREQ *)GetMsg(ahi_mp);
        if (pending)
            ReplyMsg((struct Message *)pending);
    }
    sound_number = 0;
    sound_priority = 0;
}

void SD_WaitSoundDone(void)
{
    while (SD_SoundPlaying());
}

word SD_SoundPlaying(void)
{
    if (ahi_io && ahi_ok)
        return !CheckIO((struct IORequest *)ahi_io);
    return 0;
}

void SD_StartMusic(Ptr music) {}
void SD_FadeOutMusic(void) {}

void SD_SetUserHook(void (*hook)(void)) {}

boolean SD_MusicPlaying(void) { return false; }

boolean SD_SetSoundMode(SDMode mode)
{
    SoundMode = mode;
    return TRUE;
}

boolean SD_SetMusicMode(SMMode mode)
{
    MusicMode = mode;
    return TRUE;
}

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
