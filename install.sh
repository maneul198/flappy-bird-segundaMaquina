#! /bin/bash

if [ "$1" = "clear" ];then
	rm $HOME/.config/autostart/flappyBird.desktop
	rm $HOME/.config/autostart/rotarPantalla.desktop
	rm -r $HOME/.config/flappyBirdConfig

	systemctl disable cambiarPropietarioPines.service
	systemctl disable send_email.service
	systemctl disable perroGuardian.service

	rm /usr/local/bin/cambiarPropietarioPines.sh
	rm /usr/local/bin/smart-vending-flappy-bird
	rm /usr/local/bin/powerOff-E135
	rm /usr/local/bin/send_email.sh
	rm /usr/local/bin/perroGuardian.sh


	rm /etc/systemd/system/cambiarPropietarioPines.service
	rm /etc/systemd/system/powerOff-E135.service
	rm /etc/systemd/system/send_email.service
	rm /etc/systemd/system/perroGuardian.service
	

	if [ "$2" = "--all" ];then
		rm -r */build
		echo "Se han eliminado todos los archivos de configuracion y los binarios"
		exit 0
	fi
	echo "Se han eliminado todos los archivos de configuracion"

	exit 0

fi

usuario=$(who -q | grep -v =)
cd dpci_install_3.1.1.572_release_linux_x86_64

if [ "$1" = "all" ];then

	if [ $? == 0 ];then
		./install -q
		if [ $? != 0 ];then
			echo "Ocurrio un error al tratar de instalar dpci"
		else
			echo "Se ha instalado DPCI"
		fi
	else
		echo "Error: No existe el directorio dpci"
	fi

fi

cd ..


instalar_libreria(){
	cd $1
	if [ $? == 0 ];then
		if [ ! -d build ];then
			mkdir build
		fi
	
		cd build
		cmake ..

		if [ $? == 0 ];then
			make && make install
			if [ $? != 0 ];then
				echo -e "\033[0;31mOcurrio un error al tratar de instalar" $1
				echo -e "\033[0m"
				cd ../..
				return 1
			fi
		else
			echo -e "\033[0;31mOcurrio un error al tratar de configurar" $1
			echo -e "\033[0m"
			cd ../..
			return 1
		fi

		cd ..
	fi

	chown -R $usuario build
	cd ..
	return 0
}

if [ "$1" = "f" ]; then
	instalar_libreria cmakeFlappy
else

	#instalar_libreria newEngine_sv
	instalar_libreria cmakeFlappy

fi

if [ ! -f $HOME/.config/autostart/flappyBird.desktop ];then
	cp ./flappyBird.desktop $HOME/.config/autostart/
	chown $usuario $HOME/.config/autostart/flappyBird.desktop
fi

if [ ! -f $HOME/.config/autostart/rotarPantalla.desktop ];then
	cp ./rotarPantalla.desktop $HOME/.config/autostart/
	chown $usuario $HOME/.config/autostart/rotarPantalla.desktop
fi

if [ ! -d $HOME/.config/flappyBirdConfig ];then
	cp -r ./cmakeFlappy/flappyBirdConfig $HOME/.config/
	chown -R $usuario $HOME/.config/flappyBirdConfig
fi

if [ ! -f /usr/local/bin/cambiarPropietarioPines.sh ];then
	cp ./cambiarPropietarioPines.sh /usr/local/bin/
	chown $usuario /usr/local/bin/cambiarPropietarioPines.sh
fi

if [ ! -f /usr/local/bin/perroGuardian.sh ];then
	cp ./perroGuardian.sh /usr/local/bin/
	chown $usuario /usr/local/bin/perroGuardian.sh
fi

if [ ! -f /usr/local/bin/send_email.sh ];then
	cp ./send_email.sh /usr/local/bin/
	chown $usuario /usr/local/bin/send_email.sh
fi

if [ ! -f /etc/systemd/system/cambiarPropietarioPines.service ];then
       cp ./cambiarPropietarioPines.service /etc/systemd/system/
       chown $usuario /etc/systemd/system/cambiarPropietarioPines.service
fi       

if [ ! -f /etc/systemd/system/perroGuardian.service ];then
       cp ./perroGuardian.service /etc/systemd/system/
       chown $usuario /etc/systemd/system/perroGuardian.service
fi       

if [ ! -f /etc/systemd/system/send_email.service ];then
       cp ./send_email.service /etc/systemd/system/
       chown $usuario /etc/systemd/system/send_email.service
fi           

systemctl enable cambiarPropietarioPines.service
systemctl enable send_email.service
systemctl enable perroGuardian.service
