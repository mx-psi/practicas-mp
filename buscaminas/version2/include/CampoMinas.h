#include <cstdlib>
#include <ctime>
#include <iostream> // cin, os
#include <iomanip>  // setw
#include <fstream>
#include <cstring>
#include <assert.h>
#include "Tablero.h"
using namespace std;

char* CABECERA = "#MP-BUSCAMINAS-V1";   // Cabecera de archivo de partida

/* Cabeceras de las funciones del módulo CampoMinas.cpp */

class CampoMinas{
  Tablero tab;
  bool explotado;

  bool CoordCorrectas(int x, int y) const
  {
    /* Devuelve si las coordenadas son correctas. */
    return x < tab.Filas() && y < tab.Columnas() && x >= 0 && y >= 0;
  }

  bool HayBomba(int x, int y) const
  {
    /* Devuelve si hay una bomba en la coordenada dada. */
    return CoordCorrectas(x, y) && tab(x, y).bomba;
  }

  int NumeroBombas(int x, int y) const
  {
    /* Devuelve el número de bombas en casillas adyacentes. */
    int n = 0;
    for(int i = -1; i <= 1; i++)
      for(int j = -1; j <= 1; j++)
        n += (i != 0 || j != 0) && HayBomba(x + i, y + j);
    return n;
  }

public:
  CampoMinas(int filas, int columnas, int minas)
  :tab(filas, columnas)
  {
    int minas_puestas = 0;
    srand (time(0));
    int aleatorio;
    Casilla con_bomba;
    con_bomba.bomba = true;

    while (minas_puestas < minas)
    {
      aleatorio = rand()%(filas*columnas);
      int fila = aleatorio/columnas;
      int columna = aleatorio%columnas;
      if (!tab(fila, columna).bomba)
      {
        tab(fila,  columna) =  con_bomba;
        minas_puestas++;
      }
    }
  }

  // Constructor para lectura de archivo
  CampoMinas() {}

  inline int Filas() const {return tab.Filas();}
  inline int Columnas() const {return tab.Columnas();}
  inline bool Explotado() const {return explotado;}

  bool Ganado() const
  {
    /* Indica si se ha ganado la partida. */
    for(int i = 0; i < Filas(); i++)
      for(int j = 0; j < Columnas(); j++)
        if(!tab(i,j).bomba && !tab(i,j).abierta)
          return false;

    return true;
  }

  bool Marca(int x, int y)
  {
    /* Marca o desmarca una casilla cerrada. Devuelve éxito. */
    assert(CoordCorrectas(x, y));

    if(tab(x,y).abierta)
      return false;

    tab(x, y).marcada = !tab(x, y).marcada;
    return true;
  }

  bool Abre(int x, int y){
    /* Obtiene una lista de casillas a abrir y las abre */

    if (!CoordCorrectas(x, y))
      return false;

    bool algun_cambio = false;

    struct CeldaPosicion
    {
      int fila, columna;
      CeldaPosicion* siguiente;
    };

    CeldaPosicion* pend = new CeldaPosicion;
    pend->fila = x;
    pend->columna = y;
    pend->siguiente = 0;

    while(pend != 0)
    {
      Casilla cas = tab(pend->fila, pend->columna);
      if (!cas.marcada && !cas.abierta)
      {
        algun_cambio = true;
        cas.abierta = true;
        explotado  |= cas.bomba;
        tab(pend->fila,  pend->columna) = cas;
        if (!cas.bomba && NumeroBombas(x, y) == 0)
          // Añade las casillas adyacentes
          for(int i = -1; i <= 1; i++)
            for(int j = -1; j <= 1; j++)
              if(i != 0 || j != 0)
              {
                CeldaPosicion* pend2 = new CeldaPosicion;
                pend2->fila = pend->fila + i;
                pend2->columna = pend->columna + i;
                pend2->siguiente = pend;
                delete pend;
                pend = pend2;
              }

      }
      pend = pend->siguiente;
    }

    return algun_cambio;
  }

  void PrettyPrint(ostream& os = cout) const
  {
    /* Imprime el estado actual del tablero. */

    // Columnas
    os << ' ';
    for(int i = 0; i < Columnas(); i++)
      os << setw(3) << i;
    os << std::endl;

    // Linea
    for(int i = 0; i < Columnas(); i++)
      os << "----";

    // Tablero
    for(int i = 0; i < Filas(); i++)
    {
      os << std::endl << " " << i << "|";
      for(int j = 0; j < Columnas(); j++)
      {
        Casilla actual = tab(i,j);
        if(actual.marcada)
          os << " ?|";
        else if(!actual.abierta)
          os << " *|";
        else{
          int n = NumeroBombas(i, j);
          if(n == 0)
            os << "  |";
          else
            os << n << " |";
        }
      }
    }

    // Linea
    os << std::endl;
    for(int i = 0; i < Columnas(); i++)
      os << "----";
    os << std::endl;
  }

  void ImprimeTablero(ostream& os = std::cout) const
  {
    /* Imprime el estado final del tablero. */

    if(!Ganado() && !Explotado())
      os << "No hagas trampa.";
    else
    {
      // Columnas
      os << ' ';
      for(int i = 0; i < Filas(); i++)
        os << setw(3) << i;
      os << std::endl;

      // Linea
      for(int i = 0; i < Filas(); i++)
        os << "----";

      // Tablero
      for(int i = 0; i < Filas(); i++)
      {
        os << std::endl << i << " |";
        for(int j = 0; j < Columnas(); j++)
        {
          Casilla actual = tab(i,j);
          if(actual.bomba)
            os << " X|";
          else{
            int n = NumeroBombas(i, j);
            if(n == 0)
              os << "  |";
            else
              os << n << " |";
          }
        }
      }

      // Linea
      os << std::endl;
      for(int i = 0; i < Filas(); i++)
        os << "----";
      os << std::endl;
    }
  }

  // Lectura desde archivo
  bool Leer(const char* nombre)
  {
    ifstream f(nombre);
    if (!f)
      return false;

    char* cabecera;
    f.getline(cabecera, 17);
    if (strcmp(cabecera, CABECERA))
      return false;

    return f >> tab;
  }

  // Escritura a archivo
  bool Escribir(const char* nombre)
  {
    ofstream f(nombre);
    return f << CABECERA << '\n' << tab;
  }
};