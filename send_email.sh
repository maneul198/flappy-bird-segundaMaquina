#! /bin/bash

DIR=/home/alvaro/.config/flappyBirdConfig
STRING_DAY=$(date +%Y-%m-%d)

reporte_ingresos(){
	#$1 fecha del reporte requerido
	touch $DIR/reporte
	echo >> $DIR/reporte
	echo  'Ingresos del dia:' $1 >> $DIR/reporte
	echo >> $DIR/reporte
	total_del_dia=$(grep $1 $DIR/ingresos \
		| sed s/[0-9]*-[0-9]*-[0-9]*T[0-9]*:[0-9]*:[0-9]*:\ //g\
		| paste -sd+ | bc)

	echo  "Total de ingresos:" $total_del_dia >> $DIR/reporte
	echo >> $DIR/reporte
	echo  "Logs:" >> $DIR/reporte
	grep $1 $DIR/ingresos >> $DIR/reporte
	echo >> $DIR/reporte
	echo "Entregas del dia:" $1 >> $DIR/reporte
	echo >> $DIR/reporte
	echo  "Logs:" >> $DIR/reporte
	echo >> $DIR/reporte
	grep $1 $DIR/entregasPremios >> $DIR/reporte
}

send_email(){
	echo Subject: Nomina FlappyBird Machine 2 >> $DIR/tem
	cat $DIR/$1 >> $DIR/tem
	cat $DIR/tem | ssmtp $2
	rm $DIR/tem
}

sleep 60
reporte_ingresos $STRING_DAY
echo "Reporte generado"
send_email reporte flappyBird.E135@gmail.com
echo "Reporte enviado"
rm $DIR/reporte

