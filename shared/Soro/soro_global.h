#ifndef SORO_GLOBAL_H
#define SORO_GLOBAL_H

#ifdef QT_CORE_LIB

//QObject timer macros to make shit easier
#define TIMER_INACTIVE -1
#define START_TIMER(X,Y) if (X == TIMER_INACTIVE) X = startTimer(Y)
#define KILL_TIMER(X) if (X != TIMER_INACTIVE) { killTimer(X); X = TIMER_INACTIVE; }

//shared channel names, must be the same on the rover and mission control builds
#define CHANNEL_NAME_ARM "Soro_ArmChannel"
#define CHANNEL_NAME_DRIVE "Soro_DriveChannel"
#define CHANNEL_NAME_GIMBAL "Soro_GimbalChannel"
#define CHANNEL_NAME_SHARED "Soro_SharedTcpChannel"

#ifdef QT_WIDGETS_LIB

#include <QWidget>
#include <QGraphicsDropShadowEffect>

static inline void addWidgetShadow(QWidget *target, int radius, int offset) {
    QGraphicsDropShadowEffect* ef = new QGraphicsDropShadowEffect;
    ef->setBlurRadius(radius);
    ef->setOffset(offset);
    target->setGraphicsEffect(ef);
}

#endif // QT_WIDGETS_LIB

#endif // QT_CORE_LIB

static const int MAX_VALUE_14BIT = 16383;

/* Encodes a number into 14 bits (2 bytes whrere each byte is limited to 0x7F)
 * This is done to ensure the high bit of each byte is always 0 for frame alignment purposes
 *
 * Range 0-16383
 */
static inline void serialize_14bit(unsigned short us, char *arr) {
    arr[0] = (us >> 7) & 0x7F; /* low 7 bits of second byte, high bit of first byte */
    arr[1] = us & 0x7F; //low 7 bits of first byte
}

/* Decodes a number encoded with the above function
 *
 * Range 0-16383
 */
static inline unsigned short deserialize_14bit(const char *arr) {
    return (arr[0] << 7) | arr[1];
}

template <typename T>
static inline void serialize(char *arr, T data) {
    int max = sizeof(T) - 1;
    for (int i = 0; i <= max; i++) {
        arr[max - i] = (data >> (i * 8));
    }
}

template <typename T>
static inline void deserialize(const char *arr, T& result) {
    int max = sizeof(T) - 1;
    result = (T)0x0;
    for (int i = max; i >= 0; i++) {
        result |= (arr[max - i] >> (i * 8));
    }
}

/* Converts a float ranging from -1 to 1 into an unsigned char,
 * ranging from 0 to 200
 */
static inline char joyFloatToByte(float val) {
    val = (val + 1) * 100;
    unsigned char uc = (unsigned char)val;
    return reinterpret_cast<char&>(uc);
}

/* Converts an int ranging from -100 to 100 into an unsigned char,
 * ranging from 0 to 200
 */
static inline char joyIntToByte(int val) {
    val += 100;
    unsigned char uc = (unsigned char)val;
    return reinterpret_cast<char&>(uc);
}

/* Converts an int ranging from -100 to 100 into an unsigned char,
 * ranging from 0 to 200
 */
static inline int joyFloatToInt(float val) {
    return (int)(val * 100);
}

/* Converts a byte encoded joystick axis (see joyFloatToByte) back
 * into it's original float value
 */
static inline float joyByteToFloat(char val) {
    return (float)(reinterpret_cast<unsigned char&>(val) - 100.0) / 100.0;
}

/* Converts a byte encoded joystick axis (see joyFloatToByte) into
 * an int ranging from -100 to 100
 */
static inline int joyByteToInt(char val) {
    return (int)reinterpret_cast<unsigned char&>(val) - 100;
}


#endif // SORO_GLOBAL_H
