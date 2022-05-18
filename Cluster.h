#pragma once

/*Bu baþlýk kaynaðýnýn içinde ilk baþta rastgele seçilen 2B noktalar,yeni oluþan koordinatlarý belirleyen,küme içinde bulunun nokta sayýlarýný
ifade eden deðiþkenler ve centroidlerin koordinatlarýný güncellemek için setter () ve getter () methodlarý yer alýyor.Kullanýlacak kümeler
için sýnýf oluþturuldu.*/

#ifndef K_MEANS_MIO_CPP_CLUSTER_H
#define K_MEANS_MIO_CPP_CLUSTER_H


#include <queue>
#include "Point.h"
#include <omp.h>

class Cluster {
public:
    Cluster(double x_coord, double y_coord) {
        new_x_coord = 0;
        new_y_coord = 0;
        size = 0;
        this->x_coord = x_coord;
        this->y_coord = y_coord;
    }

    Cluster() {
        new_x_coord = 0;
        new_y_coord = 0;
        size = 0;
        this->x_coord = 0;
        this->y_coord = 0;
    }

    void add_point(Point point) {
#pragma omp atomic
        new_x_coord += point.get_x_coord();
#pragma omp atomic
        new_y_coord += point.get_y_coord();
#pragma omp atomic
        size++;
    }

    void free_point() {
        this->size = 0;
        this->new_x_coord = 0;
        this->new_y_coord = 0;
    }

    double get_x_coord() {
        return this->x_coord;
    }

    double get_y_coord() {
        return this->y_coord;
    }

    bool update_coords() {


        if (this->x_coord == new_x_coord / this->size && this->y_coord == new_y_coord / this->size) {
            return false;
        }

        this->x_coord = new_x_coord / this->size;
        this->y_coord = new_y_coord / this->size;

        return true;

    }

private:
    double x_coord;
    double y_coord;
    //Nokta koordinatlarý burada yer alýyor.
    double new_x_coord;
    double new_y_coord;
    //Küme içindeki nokta sayýlarý için deðiþken 
    int size;

};


#endif 
