/*
 * menues.h
 *
 *  Created on: Aug 31, 2017
 *      Author: daniel
 *
 *  La idea es tener todos los menues creados en algun lugar de la ROM y que el main solo los valla recorriendo
 *
 *  Para Usar, Se tienen dos Estructura Memu_Menu y Menu Item:
 *  Menu_Menu tiene La descripcion del Menu o Submenu a mostrar y un link al menu anterior para poder volver
 *  Menu_Item son las opciones de cada menu, estas pueden ser o bien llamar a otro subMenu o realizar alguna accion
 *
 *  Funcionamiento:
 *  	La variable MenuTabla tiene todos los menues o submenues a mostrar. El orden dentro de este array debe coincidir
 *  con el orden que figura en el ENUM MENU_INDICE, si no no funca. Cad elemento dentyro de esta tabla contiene;
 *
 *  	Titulo del Menu, 	Texto a mostrar en la pantalla
 *  	Menu Anterior, 	A que menu vuelvo si presiono "Atras" (es un typedef enum MENU_INDICE)
 *  	Items del Menu	Apunta a un areglo de estructuras Menu_Items que constienen las distintas opciones de cada Menu
 *
 *  	Cada elemento dentro de
 *
 *
 * Antes de Usar:
 * 1º) Crear los Menues
 * 			1.1 - Por cada menu crear un areglo de Menu_Items con las opciones del menu
 *			1.2 - Crear por cada menu a mostrar en pantalla una entrada en el vector Menu_Tabla
 *			1.3 - Agregar el Menu en la lista de enum MENU_INDICE en el mismo orden que en la Tablas de Menu
 *
 *	2º) funciones o acciones a ejecutar al seleccionar un item
 *			2.1 - Agregar la declaracion de la funcion como "extern void Nombre_Funcion (void)" al comienzo del archivo
 *			2.2 - Definir o escribir la funcion en el archivo que corresponda,
 *
 * Para Usar:
 *	Desde el main llamar a la funcion:
 *				Menu_Recorrer_Opciones();
 *
 */

#ifndef MENU_INC_MENU_H_
#define MENU_INC_MENU_H_

#include <chip.h>
#include <GLCD.h>
#include <Teclado.h>
#include <task.h>

// Funcion NULL
#define Menu_NOP 0
#define Menu_Fin_Items 0

//Si pasa este lapso de tiempo sin que se presione alguna tecla, se sale del menu, volviendo al prog principal
#define Menu_Tiempo_Espera (5000/portTICK_PERIOD_MS)

/*====================
 *===			Estructuras			===
 *====================*/
//
typedef enum
{	//In_Menu ,
	In_MenuPrincipal,
	In_MenuAlarmas,
	In_MenuAjuste0,
	In_MenuSenOnOff,
	In_MenuAlarmasOnOff,
	In_FinFamilia
}MENU_INDICE;

typedef struct Menu_Menu
{
	const char* 		MenuTexto;
	const MENU_INDICE	Menu_Papa;
	struct Menu_Item*	MenuItems;
}Menu_Menu;

typedef struct Menu_Item
{
	const char* 		ItemTexto;
	const MENU_INDICE	Menu_hijo;	//Sub Menu a llamar
	void(*accion)(void * pvParam,int xNItem); //accion o funcion
}Menu_Item;

/*=====================================
 *===			Funciones			===
 *=====================================*/

uint8_t Menu_Mostrar_Menu(const Menu_Menu *pMenuMenu,uint8_t ventana);
uint8_t Menu_Contar_Items(const Menu_Menu *pMenuMenu);
void Menu_Mostrar_Menu_Botones (uint8_t botDown,uint8_t botUP);
void Menu_Recorrer_Opciones(void);

void MenuFunInit	 (void);
void MenuFunAjuste0  (void * pvParam,int xNItem);
void MenuFunAlarmas01(void * pvParam,int xNItem);
void MenuFunAlarmaOnOff(void * pvParam,int xNItem);
void MenuFunOnOff	 (void * pvParam,int xNItem);

/*=================================
 *===			Menues			===
 *=================================*/

//Estan en ROM o es la idea.. serian como los fuentes

//Items Menu Principal
const Menu_Item MenuPrincipal_I[]={
		{"ALARMAS",In_MenuAlarmas,Menu_NOP},
		{"AJUSTE de 0",In_MenuAjuste0,Menu_NOP},
		{"SENSORES ON/OFF",In_MenuSenOnOff,Menu_NOP},
		{Menu_Fin_Items,In_FinFamilia,Menu_NOP}
};


const Menu_Item  MenuAlarmas_I[]={
		{"Temp Alarma",In_FinFamilia,MenuFunAlarmas01},
		{"Sel Sensores",In_MenuAlarmasOnOff,Menu_NOP},
		{Menu_Fin_Items,In_FinFamilia,Menu_NOP}
};

const Menu_Item MenuAlarmasOnOff_I[]={
		{"Sensor 1 ",In_FinFamilia,MenuFunAlarmaOnOff},
		{"Sensor 2 ",In_FinFamilia,MenuFunAlarmaOnOff},
		{"Sensor 3 ",In_FinFamilia,MenuFunAlarmaOnOff},
		{"Sensor 4 ",In_FinFamilia,MenuFunAlarmaOnOff},
		{"Sensor 5 ",In_FinFamilia,MenuFunAlarmaOnOff},
		{"Sensor 6 ",In_FinFamilia,MenuFunAlarmaOnOff},
		{Menu_Fin_Items,In_FinFamilia,Menu_NOP},
};

const Menu_Item MenuAjuste0_I[]={
		{"Sensor 1 ",In_FinFamilia,MenuFunAjuste0},
		{"Sensor 2 ",In_FinFamilia,MenuFunAjuste0},
		{"Sensor 3 ",In_FinFamilia,MenuFunAjuste0},
		{"Sensor 4 ",In_FinFamilia,MenuFunAjuste0},
		{"Sensor 5 ",In_FinFamilia,MenuFunAjuste0},
		{"Sensor 6 ",In_FinFamilia,MenuFunAjuste0},
		{Menu_Fin_Items,In_FinFamilia,MenuFunAjuste0}
};

const Menu_Item MenuSenOnOff_I[]={
		{"Sensor 1 ",In_FinFamilia,MenuFunOnOff},
		{"Sensor 2 ",In_FinFamilia,MenuFunOnOff},
		{"Sensor 3 ",In_FinFamilia,MenuFunOnOff},
		{"Sensor 4 ",In_FinFamilia,MenuFunOnOff},
		{"Sensor 5 ",In_FinFamilia,MenuFunOnOff},
		{"Sensor 6 ",In_FinFamilia,MenuFunOnOff},
		{Menu_Fin_Items,In_FinFamilia,Menu_NOP},
};

const Menu_Menu  MenuTabla[]={
		{"MENU",In_FinFamilia,(Menu_Item*)MenuPrincipal_I},
		{"ALARMAS",In_MenuPrincipal,(Menu_Item*)MenuAlarmas_I},
		{"AJUSTE DE 0",In_MenuPrincipal,(Menu_Item*)MenuAjuste0_I},
		{"ACTIVAR SENSORES",In_MenuPrincipal,(Menu_Item*)MenuSenOnOff_I},
		{"ALARMAS ON/OFF",In_MenuAlarmas,(Menu_Item*)MenuAlarmasOnOff_I},
};

/*=====================================
 *===			Funciones			===
 *=====================================*/
/*************************************
 * 		Menu_Mostrar_Menu
 * 	Muestra en pantalla los mensajes asociados a cada nuevo Menu
 * 	Muestra el titulo+Tres Items del Menu+Menu de Botones.
 * 	Si hay mas de tres items los muetra en la siguiente "Ventana"
 */
uint8_t Menu_Mostrar_Menu(const Menu_Menu *pMenuMenu,uint8_t ventana)
{
	Menu_Item * pMenuItem;
	uint8_t i=0;

	pMenuItem=(Menu_Item*)(pMenuMenu->MenuItems);
	GLCD_Borrar_Display(Letras_Blancas);

	//Titulo del Menu
	GLCD_Escribir_Centrado((uint8_t*)(pMenuMenu->MenuTexto),1,Letras_Blancas);

	pMenuItem+=(ventana*3);
	//Camino y muestro los Items del Menu
	//finalizo cuando el texto del menu sea 0
	while((pMenuItem->ItemTexto) && (i<3))
	{
		GLCD_Escribir_Frase((uint8_t*) (pMenuItem->ItemTexto),18,(3+i),Letras_Blancas);
		pMenuItem++;
		i++;
	}

	//Muestro menu inferior o pa que es cada boton
	Menu_Mostrar_Menu_Botones(97,96);
	return(i-1);
}

/*************************************
 * 		Menu_Contar_Items
 * 	Devuelve la cantidad de items que contiene el menu a mostrar
 */
uint8_t Menu_Contar_Items(const Menu_Menu *pMenuMenu)
{
	uint8_t i=0;
	while(( ((pMenuMenu->MenuItems)+i)->ItemTexto) )
	{
		i++;
	}
	return(i-1);
}

/*************************************
 * 		Menu_Recorrer_Opciones
 * Se encarga de recorrer la lisat de los menues y submenues,
 * utiliza la funcion mostrar_menu para visualizar en pantalla
 * las distintas opciones.
 * Controla el funcionamiento de las teclas OK y Atras
 */
void Menu_Recorrer_Opciones(void)
{
	uint8_t tecla=BotonUp;
	uint8_t seleccion=0,Maximo_Submenu;
	const Menu_Menu *pMenuMenu;
	Menu_Item * pMenuItem;

	pMenuMenu=MenuTabla;
	Maximo_Submenu=Menu_Contar_Items(pMenuMenu);
	pMenuItem=(Menu_Item*)(pMenuMenu->MenuItems);

	//Muestro el Menu en Pantalla
	while(tecla!=BotonFantasma)
	{
		//Quedo a la espera de una tecla o tiempo maximo
		Maximo_Submenu=Menu_Contar_Items(pMenuMenu);
		Menu_Mostrar_Menu(pMenuMenu,(seleccion/3));
		GLCD_Escribir_Caracter_Tabla(94,6,(3+(seleccion%3)),Letras_Negras);//Flecha de Seleccion
		tecla=Teclado_Esperar_Tecla(Menu_Tiempo_Espera);

		if(tecla==BotonDown)
			{
				GLCD_Escribir_Frase((uint8_t*)" ",6,(3+(seleccion%3)),Letras_Negras);
				if(seleccion<Maximo_Submenu)
					seleccion++;
				else
					seleccion=0;
			}
		if(tecla==BotonUp)
			{
				GLCD_Escribir_Frase((uint8_t*)" ",6,(3+(seleccion%3)),Letras_Negras);
				if(seleccion>0)
					seleccion--;
				else
					seleccion=Maximo_Submenu;
			}

		if(tecla==BotonOk)
			{
				pMenuItem+=seleccion; // p Apunta al Item del Menu que seleccione
				if((pMenuItem->Menu_hijo)==In_FinFamilia)
				{// Es una Opcion, hay que llamar a una funcion
					pMenuItem->accion(( void *)pMenuItem,seleccion);
					pMenuItem=(Menu_Item*)(pMenuMenu->MenuItems);
				}
				else
				{//Es un Sub menu,
					if((pMenuItem->accion)!=Menu_NOP)
					{//hay que llamar a un afuncion antes de cambiar de menu
						pMenuItem->accion(( void *)pMenuItem,seleccion);
					}
					pMenuMenu=(MenuTabla+(pMenuItem->Menu_hijo));
					pMenuItem=(Menu_Item*)(pMenuMenu->MenuItems);
					seleccion=0;
				}
			}

		if(tecla==BotonMenu)
			{//Hay que volve pa atras
				if((pMenuMenu->Menu_Papa)==In_FinFamilia)
				{//Estabamso en el Padre, Salimos de las opciones de Menu
					tecla=BotonFantasma;
				}
				else
				{//Volvemos al Menu anterior
					pMenuMenu=(MenuTabla+(pMenuMenu->Menu_Papa));
					pMenuItem=(Menu_Item*)(pMenuMenu->MenuItems);
					seleccion=0;
				}
			}

	}//while(tecla!=BotonFantasma)
	GLCD_Borrar_Display(Letras_Negras);
	GLCD_Escribir_Centrado((uint8_t*)"Saliendo del Menu",1,Letras_Negras);
	GLCD_Escribir_Centrado((uint8_t*)"Configurando Opciones",3,Letras_Negras);
	vTaskDelay(500/portTICK_PERIOD_MS);
	GLCD_Escribir_Centrado((uint8_t*)"[  ...  ]",5,Letras_Negras);
	vTaskDelay(500/portTICK_PERIOD_MS);
	GLCD_Escribir_Centrado((uint8_t*)"[ ..... ]",5,Letras_Negras);
	vTaskDelay(500/portTICK_PERIOD_MS);
	GLCD_Escribir_Centrado((uint8_t*)"[.......]",5,Letras_Negras);
	vTaskDelay(500/portTICK_PERIOD_MS);
	GLCD_Borrar_Display(Letras_Negras);
}

/*************************************
 * 		Menu_Mostrar_Menu_Botones
 * Arma el menu con el significado de cada boton segun la Funcion
 * Recibe el caracter de la tabala a mostrar. 96,97 para Arri/Aba
 */
void Menu_Mostrar_Menu_Botones (uint8_t botDown,uint8_t botUP)
{
	uint8_t aux[]={" BACK | OK | 1 | 1 "};//96+32,0}
	aux[13]=(botDown+32);
	aux[17]=(botUP+32);

	GLCD_Borrar_Fila(7,Letras_Blancas);
	GLCD_Escribir_Centrado((uint8_t*)aux,7,Letras_Negras);
}

#endif /* MENU_INC_MENU_H_ */
