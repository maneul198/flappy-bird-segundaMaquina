#include "joystick_flappy.h"

joystick_flappy::joystick_flappy(QObject *parent):
    QThread(parent)
{
    button= NULL;
    axis= NULL;
    num_of_axis= NULL;
    num_of_buttons= 0;
    a= true;
}

int joystick_flappy::abrir_joystick(){
    if( ( joy_fd = open(JOY_DEV, O_RDONLY)) == -1){
        return -1;
    }

    ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
    ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
    ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );

    axis = (int *) calloc( num_of_axis, sizeof( int ) );
    button = (char *) calloc( num_of_buttons, sizeof( char ) );

    fcntl( joy_fd, F_SETFL, O_NONBLOCK );


    return 1;
}

void joystick_flappy::run()
{
    forever{
        read (joy_fd, &js , sizeof(struct js_event));
        switch (js.type & ~JS_EVENT_INIT)
        {
        /*
            case JS_EVENT_AXIS:
                axis   [ js.number ] = js.value;
                break;
         */
            case JS_EVENT_BUTTON:
                button [ js.number ] = js.value;
                //qDebug() << js.number << js.value << endl;

                if(js.value == 0 && !a){
                    a= true;

                }else if(js.value == 1 && a){
                    emit buttom_pressed(js.number);
                    //qDebug() << "SE EMITE SEÑAL" << endl;
                    a= false;
                }


                break;
        }
        //msleep(300);
    }


}
