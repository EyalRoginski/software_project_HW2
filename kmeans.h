#ifndef KMEANS_H
#define KMEANS_H

struct coordinate {
    struct coordinate *next;
    double value;
};

struct vector {
    struct vector *next;
    struct coordinate *coordinates;
};

struct vector *kmeans(int N, int K, int iter, struct vector *vectors,
    struct vector *centroids, double epsilon);

void free_vector(struct vector *vec);
void free_coordinate(struct coordinate *coord);

#endif
