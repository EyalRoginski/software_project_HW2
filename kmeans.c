#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CLUSTER_ERROR "Invalid number of clusters!"
#define ITER_ERROR "Invalid maximum iteration!"
#define GENERIC_ERROR "An Error Has Occurred"

#define ITER_DEFAULT 200
#define EPSILON 0.001

#define CLUSTERS(N, i, j) N *j + i

struct coordinate {
    struct coordinate *next;
    double value;
};

struct vector {
    struct vector *next;
    struct coordinate *coordinates;
};

void print_coord(struct coordinate *coord);

void error(const char *error_message)
{
    printf("%s\n", error_message);
    exit(1);
}

void get_args(int argc, char *argv[], int *K, int *iter)
{
    if (argc != 2 && argc != 3) {
        error(GENERIC_ERROR);
    }

    *K = atoi(argv[1]);
    if (*K <= 1) {
        error(CLUSTER_ERROR);
    }

    if (argc == 3) {
        *iter = atoi(argv[2]);
        if (*iter <= 1 || *iter >= 1000) {
            error(ITER_ERROR);
        }
    } else {
        /* Iter not provided */
        *iter = ITER_DEFAULT;
    }
}

struct coordinate *get_coordinates()
{
    struct coordinate *coord_head;
    struct coordinate *current;
    struct coordinate *new_coord;

    coord_head = malloc(sizeof(struct coordinate));
    coord_head->next = NULL;
    if (scanf("%lf", &coord_head->value) != 1) {
        free(coord_head);
        return NULL;
    }

    current = coord_head;

    while (getchar() == ',') {
        new_coord = malloc(sizeof(struct coordinate));
        if (scanf("%lf", &new_coord->value) != 1) {
            error(GENERIC_ERROR);
        }

        current->next = new_coord;
        new_coord->next = NULL;
        current = new_coord;
    }

    return coord_head;
}

struct vector *get_vectors()
{
    struct vector *head;
    struct vector *current;
    struct vector *new_vector;
    struct coordinate *coords;

    coords = get_coordinates();
    head = malloc(sizeof(struct vector));
    head->next = NULL;
    head->coordinates = coords;

    current = head;

    while ((coords = get_coordinates()) != NULL) {
        new_vector = malloc(sizeof(struct vector));
        new_vector->coordinates = coords;
        new_vector->next = NULL;

        current->next = new_vector;
        current = new_vector;
    }

    return head;
}

int get_vector_list_length(struct vector *list)
{
    int count = 0;
    while (list != NULL) {
        count += 1;
        list = list->next;
    }
    return count;
}

int get_coords_dimension(struct coordinate *coords)
{
    int count = 0;
    while (coords != NULL) {
        count += 1;
        coords = coords->next;
    }
    return count;
}

void copy_coordinates(struct coordinate **dest, struct coordinate *source)
{
    while (source != NULL) {
        *dest = malloc(sizeof(struct coordinate));
        (*dest)->next = source->next;
        (*dest)->value = source->value;
        source = source->next;
        dest = &((*dest)->next);
    }
}

struct vector *init_centroids_from_vectors(struct vector *vectors, int K)
{
    struct vector *centroid_head;
    struct vector *current_centroid;
    struct vector *current_vector;
    struct vector *new_centroid;
    int i;

    centroid_head = malloc(sizeof(struct vector));
    centroid_head->next = NULL;

    current_centroid = centroid_head;
    current_vector = vectors;
    copy_coordinates(&current_centroid->coordinates, current_vector->coordinates);
    current_vector = current_vector->next;

    for (i = 1; i < K; i++) {
        new_centroid = malloc(sizeof(struct vector));

        copy_coordinates(&new_centroid->coordinates, current_vector->coordinates);
        new_centroid->next = NULL;

        current_centroid->next = new_centroid;
        current_centroid = new_centroid;

        current_vector = current_vector->next;
    }

    return centroid_head;
}

double distance(struct coordinate *coordinates1, struct coordinate *coordinates2)
{
    double sum = 0;

    while (coordinates1 != NULL) {
        sum += (coordinates1->value - coordinates2->value) * (coordinates1->value - coordinates2->value);
        coordinates1 = coordinates1->next;
        coordinates2 = coordinates2->next;
    }

    return sqrt(sum);
}

int get_closest_centroid_index(struct coordinate *coordinates, struct vector *centroids)
{
    int i = 0;
    int closest_centroid_index = 0;
    double d = 0;
    double shortest_distance;

    shortest_distance = distance(centroids->coordinates, coordinates);

    while (centroids != NULL) {
        d = distance(centroids->coordinates, coordinates);

        if (d < shortest_distance) {
            shortest_distance = d;
            closest_centroid_index = i;
        }

        centroids = centroids->next;
        i++;
    }

    return closest_centroid_index;
}

void remove_vector_from_all_clusters(int N, int K, int *clusters, int vector_index)
{
    int i;
    for (i = 0; i < K; i++) {
        clusters[CLUSTERS(N, vector_index, i)] = 0;
    }
}

void add_vector_to_cluster(int N, int *clusters, int vector_index, int cluster_index)
{
    clusters[CLUSTERS(N, vector_index, cluster_index)] = 1;
}

int is_vector_in_cluster(int N, int *clusters, int vector_index, int cluster_index)
{
    return clusters[CLUSTERS(N, vector_index, cluster_index)];
}

void assign_vectors_to_closest_centroid(int N, int K, struct vector *vectors,
    struct vector *centroids, int *clusters)
{
    struct vector *current_vector;
    int vector_index;
    int closest_centroid_index;

    current_vector = vectors;
    for (vector_index = 0; vector_index < N; vector_index++) {
        closest_centroid_index = get_closest_centroid_index(current_vector->coordinates, centroids);

        remove_vector_from_all_clusters(N, K, clusters, vector_index);
        add_vector_to_cluster(N, clusters, vector_index, closest_centroid_index);

        current_vector = current_vector->next;
    }
}

void add_coord_to_coord(struct coordinate *dest, struct coordinate *source)
{
    while (dest != NULL) {
        dest->value += source->value;
        dest = dest->next;
        source = source->next;
    }
}

void divide_coord_by_double(struct coordinate *coord, double divider)
{
    while (coord != NULL) {
        coord->value /= divider;
        coord = coord->next;
    }
}

void zero_coord(struct coordinate *coord)
{
    while (coord != NULL) {
        coord->value = 0;
        coord = coord->next;
    }
}

void free_coordinate(struct coordinate *coord)
{
    struct coordinate *last_cord;
    last_cord = coord;
    while (coord != NULL) {
        coord = coord->next;
        free(last_cord);
        last_cord = coord;
    }
}

void free_vector(struct vector *vec)
{
    struct vector *last_vector;
    last_vector = vec;
    while (vec != NULL) {
        vec = vec->next;
        free_coordinate(last_vector->coordinates);
        free(last_vector);
        last_vector = vec;
    }
}

double update_centroid(int N, struct vector *vectors,
    struct vector *centroid, int *clusters, int centroid_index)
{
    struct coordinate *old_coordinates;
    struct vector *vector;
    int vector_index;
    int cluster_size;
    double delta;

    copy_coordinates(&old_coordinates, centroid->coordinates);
    zero_coord(centroid->coordinates);

    cluster_size = 0;
    vector = vectors;

    for (vector_index = 0; vector_index < N; vector_index++) {
        if (is_vector_in_cluster(N, clusters, vector_index, centroid_index)) {
            add_coord_to_coord(centroid->coordinates, vector->coordinates);
            cluster_size++;
        }

        vector = vector->next;
    }

    divide_coord_by_double(centroid->coordinates, 1.0 * cluster_size);

    delta = distance(centroid->coordinates, old_coordinates);

    free_coordinate(old_coordinates);
    return delta;
}

double update_centroids(int N, int K, struct vector *vectors,
    struct vector *centroids, int *clusters)
{
    double max_delta;
    int i;
    max_delta = 0;
    for (i = 0; i < K; i++) {
        double delta = update_centroid(N, vectors, centroids, clusters, i);

        if (max_delta < delta) {
            max_delta = delta;
        }

        centroids = centroids->next;
    }

    return max_delta;
}

struct vector *kmeans(int N, int K, int iter, struct vector *vectors, struct vector *centroids, double epsilon)
{
    int *clusters;
    int i;

    clusters = malloc(sizeof(int) * N * K); /* an N x K matrix */
    for (i = 0; i < N * K; i++) {
        clusters[i] = 0;
    }

    for (i = 0; i < iter; i++) {
        double max_delta;
        assign_vectors_to_closest_centroid(N, K, vectors, centroids, clusters);

        max_delta = update_centroids(N, K, vectors, centroids, clusters);
        if (max_delta <= epsilon) {
            break;
        }
    }

    free(clusters);
    return centroids;
}

void print_coord(struct coordinate *coord)
{
    while (coord != NULL) {
        if (coord->next == NULL) {
            printf("%.4f\n", coord->value);
        } else {
            printf("%.4f,", coord->value);
        }
        coord = coord->next;
    }
}

void print_centroids(struct vector *centroids)
{
    while (centroids != NULL) {
        print_coord(centroids->coordinates);
        centroids = centroids->next;
    }
}
