#include <iostream>
#include <cmath>
#include <fstream>
#include <chrono>
#include "Point.h"
#include "Cluster.h"
#include <omp.h>

using namespace std;
using namespace std::chrono;

double max_range = 100000;
int num_point = 100000;
int num_cluster = 10;
int max_iterations = 20;

vector<Point> init_point(int num_point);
vector<Cluster> init_cluster(int num_cluster);
void compute_distance(vector<Point>& points, vector<Cluster>& clusters);
double euclidean_dist(Point point, Cluster cluster);
bool update_clusters(vector<Cluster>& clusters);
void draw_chart_gnu(vector<Point>& points);

int main() {

    printf("Nokta Sayisi %d\n", num_point);
    printf("Kume sayisi %d\n", num_cluster);
    //omp_get_num_procs() : ��lev �a�r�ld���nda kullan�labilir olan i�lemcilerin say�s�n� d�nd�r�r.
    printf("Islemci sayisi: %d\n", omp_get_num_procs());

    srand(int(time(NULL)));
    //omp_get_wtime() : Belirli bir noktadan ge�en s�renin saniye cinsinden bir de�er d�nd�r�r.
    double time_point1 = omp_get_wtime();
    printf("Olusturmalar basliyor..\n");

    vector<Point> points;
    vector<Cluster> clusters;

    //Paralel k�me ve nokta ba�latmas� ger�ekle�tiriliyor.
#pragma omp parallel
    {
#pragma omp sections
        {
#pragma omp section
            {
                printf("Noktalar olusturuldu..\n");
                points = init_point(num_point);
                printf("Noktalar baslatiliyor \n");
            }
#pragma omp section
            {
                printf("Kumeler olusturuldu..\n");
                clusters = init_cluster(num_cluster);
                printf("Kumeler baslatiliyor\n");
            }
        }
    }

    double time_point2 = omp_get_wtime();
    double duration = time_point2 - time_point1;

    printf("Noktalar ve Kumeler: %f saniyede olusturuldu \n", duration);

    bool conv = true;
    int iterations = 0;

    printf("Iterasyonlar...\n");

    //�terasyonlar maks_iterasyondan b�y�k oldu�unda veya k�meler hareket etmedi�inde durur.
    while (conv && iterations < max_iterations) {

        iterations++;

        compute_distance(points, clusters);

        conv = update_clusters(clusters);

        printf("Iterasyon %d bitti \n", iterations);

    }

    double time_point3 = omp_get_wtime();
    duration = time_point3 - time_point2;

    printf("Iterasyon sayisi: %d, toplam zaman: %f saniye, iterasyon basina dusen sure: %f saniye\n",
        iterations, duration, duration / iterations);

    try {
        printf("Grafik ciziliyor...\n");
        draw_chart_gnu(points);
    }
    catch (int e) {
        printf("Grafik cizilemedi,gnuplot bulunamadi");
    }

    return 0;


}

//num_point ile belirtilen noktalar� ba�lat�yor
vector<Point> init_point(int num_point) {

    vector<Point> points(num_point);
    Point* ptr = &points[0];


    for (int i = 0; i < num_point; i++) {

        Point* point = new Point(rand() % (int)max_range, rand() % (int)max_range);

        ptr[i] = *point;

    }

    return points;

}

//num_cluster ile belirtilen k�meleri ba�lat�yor
vector<Cluster> init_cluster(int num_cluster) {

    vector<Cluster> clusters(num_cluster);
    Cluster* ptr = &clusters[0];

    for (int i = 0; i < num_cluster; i++) {

        Cluster* cluster = new Cluster(rand() % (int)max_range, rand() % (int)max_range);

        ptr[i] = *cluster;

    }

    return clusters;
}

// Her nokta i�in, her k�me aras�ndaki mesafeyi hesaplay�p  noktay� en yak�n k�meye at�yor
//Mesafeyi �klid mesafesi ile hesapl�yor
// D��taki for, private = min_distance, min_index, points_size, clusters_size ve clustes ile paraleldir.
/* Noktalar�n vekt�r� payla��l�r. Nokta ba��na ger�ekle�tirilen hesaplama miktar� sabittir,
    bu nedenle statik i� par�ac��� zamanlamas� se�ilmi�tir*/

void compute_distance(vector<Point>& points, vector<Cluster>& clusters) {
    unsigned long points_size = points.size();
    unsigned long clusters_size = clusters.size();

    double min_distance;
    int min_index;

#pragma omp parallel default(shared) private(min_distance, min_index) firstprivate(points_size, clusters_size)
    {
#pragma omp for schedule(static)//statik zamanlama kullan�ld�.Her i� par�ac��� i�in e�it.
        for (int i = 0; i < points_size; i++) {

            Point& point = points[i];

            min_distance = euclidean_dist(point, clusters[0]);
            min_index = 0;

            for (int j = 1; j < clusters_size; j++) {

                Cluster& cluster = clusters[j];

                double distance = euclidean_dist(point, cluster);

                if (distance < min_distance) {

                    min_distance = distance;
                    min_index = j;
                }

            }
            point.set_cluster_id(min_index);
#pragma omp critical
            clusters[min_index].add_point(point);
        }
    }
}

double euclidean_dist(Point point, Cluster cluster) {

    double distance = sqrt(pow(point.get_x_coord() - cluster.get_x_coord(), 2) +
        pow(point.get_y_coord() - cluster.get_y_coord(), 2));

    return distance;
}

//Her k�me i�in koordinatlar� g�ncelliyor. Yaln�zca bir k�me hareket ederse d�ng�den ��k�l�r d�n�� de�eri true olur
// lastprivate = conv ile her k�me i�in bir paralel se�ildi
bool update_clusters(vector<Cluster>& clusters) {

    bool conv = false;

    for (int i = 0; i < clusters.size(); i++) {
        conv = clusters[i].update_coords();
        clusters[i].free_point();
    }

    return conv;
}

//Gnuplot ile nokta grafi�i �izme
void draw_chart_gnu(vector<Point>& points) {

    ofstream outfile("data.txt");

    for (int i = 0; i < points.size(); i++) {

        Point point = points[i];
        outfile << point.get_x_coord() << " " << point.get_y_coord() << " " << point.get_cluster_id() << std::endl;

    }

    outfile.close();
    system("gnuplot -p -e \"plot 'data.txt' using 1:2:3 with points palette notitle\"");
    remove("data.txt");

}
