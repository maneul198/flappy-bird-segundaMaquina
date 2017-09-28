
#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <QObject>
#include <QThread>
#include <QDebug>

#define JOY_DEV "/dev/input/js0"

class joystick_flappy : public QThread
{
    Q_OBJECT
    void run() override;

public:
    joystick_flappy(QObject *parent = 0);
    int abrir_joystick();

signals:
    void buttom_pressed(int button);
    void joystick_change(int num);

private:
    struct js_event js;
    int joy_fd;
    int num_of_axis;
    int *axis;
    int num_of_buttons;
    char *button;
    char name_of_joystick[80];
    bool a;

    /*
    int joy_fd, *axis=NULL, num_of_axis=0, num_of_buttons=0, x;
    char *button=NULL, name_of_joystick[80];
    struct js_event js;

    if( ( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 )
    {
        printf( "Couldn't open joystick\n" );
        return -1;
    }

    ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
    ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
    ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );

    axis = (int *) calloc( num_of_axis, sizeof( int ) );
    button = (char *) calloc( num_of_buttons, sizeof( char ) );

    printf("Joystick detected: %s\n\t%d axis\n\t%d buttons\n\n"
        , name_of_joystick
        , num_of_axis
        , num_of_buttons );

    fcntl( joy_fd, F_SETFL, O_NONBLOCK );

    while( 1 )
    {

        read(joy_fd, &js, sizeof(struct js_event));

        switch (js.type & ~JS_EVENT_INIT)
        {
            case JS_EVENT_AXIS:
                axis   [ js.number ] = js.value;
                break;
            case JS_EVENT_BUTTON:
                button [ js.number ] = js.value;
                break;
        }

        printf( "X: %6d  Y: %6d  ", axis[0], axis[1] );

        if( num_of_axis > 2 )
            printf("Z: %6d  ", axis[2] );

        if( num_of_axis > 3 )
            printf("R: %6d  ", axis[3] );

        for( x=0 ; x<num_of_buttons ; ++x )
            printf("B%d: %d  ", x, button[x] );

        printf("  \r");
        fflush(stdout);
    }

    close( joy_fd );
    return 0;
*/
};
#endif
