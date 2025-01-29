#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdlib.h>
#include "kmeans.h"

static PyObject *fit(PyObject *self, PyObject *args);

const char fit_docstring[] = "Apply the kmeans algorithm with given starting centroids.\n"
                             "Arguments:\n"
                             " - iter: int\n"
                             " - epsilon: float\n"
                             " - vectors: list[list[float]]\n"
                             " - centroids: list[list[float]]\n"
                             "\n"
                             "@return list[float[float]] the centroids after applying the kmeans algorithm.\n";

static PyMethodDef kmeans_methods[]
    = {
          { "fit",
              (PyCFunction)fit,
              METH_VARARGS,
              PyDoc_STR(fit_docstring) },
          { NULL, NULL, 0, NULL }
      };

static struct PyModuleDef kmeansmodule = {
    PyModuleDef_HEAD_INIT,
    "mykmeanssp",
    NULL,
    -1,
    kmeans_methods
};

PyMODINIT_FUNC PyInit_mykmeanssp(void)
{
    PyObject *m;
    m = PyModule_Create(&kmeansmodule);
    if (!m) {
        return NULL;
    }
    return m;
}

/**
 * Creates a new coordinate structure.
 *
 * @param value The double value to store in the coordinate.
 * @return A pointer to the newly created coordinate, or NULL if memory allocation fails.
 */
struct coordinate *create_coordinate(double value)
{
    struct coordinate *new_coord = (struct coordinate *)malloc(sizeof(struct coordinate));
    if (new_coord == NULL) {
        return NULL; // Memory allocation failure
    }
    new_coord->value = value;
    new_coord->next = NULL;
    return new_coord;
}

/**
 * Creates a new vector structure.
 *
 * @return A pointer to the newly created vector, or NULL if memory allocation fails.
 */
struct vector *create_vector()
{
    struct vector *new_vec = (struct vector *)malloc(sizeof(struct vector));
    if (new_vec == NULL) {
        return NULL; // Memory allocation failure
    }
    new_vec->coordinates = NULL;
    new_vec->next = NULL;
    return new_vec;
}

/**
 * Converts a Python list[float] into a struct vector.
 *
 * This function creates a linked list of coordinates from the Python list and stores
 * it in a new vector structure.
 *
 * @param py_list A Python list object containing floats.
 * @return A pointer to the newly created vector, or NULL if:
 *         - The input is not a Python list.
 *         - An element in the list is not a float.
 *         - Memory allocation fails.
 */
struct vector *vector_from_py_list(PyObject *py_list)
{
    if (!PyList_Check(py_list)) {
        return NULL; // Input is not a list
    }

    struct vector *new_vector = create_vector();
    if (new_vector == NULL) {
        return NULL; // Memory allocation failure
    }

    Py_ssize_t num_coordinates = PyList_Size(py_list);
    struct coordinate *current_coordinate = NULL;

    for (Py_ssize_t j = 0; j < num_coordinates; j++) {
        PyObject *py_coord = PyList_GetItem(py_list, j);
        if (!PyFloat_Check(py_coord)) {
            return NULL; // Element is not a float
        }

        double value = PyFloat_AsDouble(py_coord);
        struct coordinate *new_coord = create_coordinate(value);
        if (new_coord == NULL) {
            return NULL; // Memory allocation failure
        }

        if (new_vector->coordinates == NULL) {
            new_vector->coordinates = new_coord;
        } else {
            current_coordinate->next = new_coord;
        }
        current_coordinate = new_coord;
    }

    return new_vector;
}

/**
 * Converts a Python list[list[float]] into a linked list of struct vectors.
 *
 * This function iterates over the outer list, converting each inner list[float]
 * into a struct vector using the `vector_from_py_list` function, and links them together.
 *
 * @param list A Python list object containing lists of floats.
 * @return A pointer to the head of the linked list of vectors, or NULL if:
 *         - The input is not a Python list.
 *         - An element in the outer list is not a list.
 *         - An element in any inner list is not a float.
 *         - Memory allocation fails.
 */
struct vector *vectors_from_py_list_list(PyObject *list)
{
    if (!PyList_Check(list)) {
        return NULL; // Input is not a list
    }

    Py_ssize_t num_vectors = PyList_Size(list);
    struct vector *head = NULL;
    struct vector *current_vector = NULL;

    for (Py_ssize_t i = 0; i < num_vectors; i++) {
        PyObject *py_vector = PyList_GetItem(list, i);
        if (!PyList_Check(py_vector)) {
            return NULL; // Element is not a list
        }

        struct vector *new_vector = vector_from_py_list(py_vector);
        if (new_vector == NULL) {
            return NULL; // Error creating vector from list
        }

        if (head == NULL) {
            head = new_vector;
        } else {
            current_vector->next = new_vector;
        }
        current_vector = new_vector;
    }

    return head;
}

/**
 * Converts a linked list of `struct coordinate` into a Python list of floats.
 *
 * @param coord A pointer to the head of a linked list of `struct coordinate`.
 *
 * @return A PyObject representing a Python list of floats. Each float corresponds
 *         to the `value` field of a `struct coordinate` in the linked list.
 *         Returns NULL on error.
 */
PyObject *list_float_from_coordinates(struct coordinate *coord)
{
    // Create the inner Python list for coordinates
    PyObject *inner_list = PyList_New(0);
    if (inner_list == NULL) {
        return NULL; // Memory allocation failed
    }

    struct coordinate *current_coordinate = coord;
    while (current_coordinate != NULL) {
        // Create a Python float from the double value
        PyObject *py_float = Py_BuildValue("d", current_coordinate->value);
        if (py_float == NULL) {
            return NULL; // Memory allocation failed
        }

        // Append the float to the inner list
        if (PyList_Append(inner_list, py_float) < 0) {
            return NULL; // Append failed
        }

        current_coordinate = current_coordinate->next;
    }

    return inner_list; // Return the final Python object
}

/**
 * Converts a linked list of vectors into a Python list of lists of floats.
 *
 * Each vector contains a linked list of coordinates, which is converted into a Python
 * list of floats. The outer linked list of vectors is converted into a Python list,
 * where each element is a Python list representing the coordinates of a vector.
 *
 * @param vec A pointer to the head of a linked list of vectors.
 *
 * @return A PyObject representing a Python list of lists of floats. Each inner list
 *         corresponds to the coordinates of a vector in the linked list.
 *         Returns NULL if memory allocation fails.
 */
PyObject *list_list_float_from_vectors(struct vector *vec)
{
    // Create the outer Python list
    PyObject *outer_list = PyList_New(0);
    if (outer_list == NULL) {
        return NULL; // Memory allocation failed
    }

    struct vector *current_vector = vec;
    while (current_vector != NULL) {
        // Create the inner Python list for coordinates
        PyObject *inner_list = list_float_from_coordinates(current_vector->coordinates);
        if (inner_list == NULL) {
            return NULL; // Inner list creation failed
        }

        // Append the inner list to the outer list
        if (PyList_Append(outer_list, inner_list) < 0) {
            return NULL; // Append failed
        }

        current_vector = current_vector->next;
    }

    return outer_list; // Return the final Python object
}

/**
 * A python-wrapper function for kmeans.
 *
 * @param args Represents the arguments given in python, which should be:
 * - iter: int
 * - epsilon: float
 * - py_vectors: list[list[float]]
 * - py_centroids: list[list[float]]
 *
 * @return A PyObject representing the centroids after applying the kmeans algorithm.
 * */
static PyObject *fit(PyObject *self, PyObject *args)
{
    int iter;
    double epsilon;
    PyObject *py_vectors;
    PyObject *py_centroids;

    if (!PyArg_ParseTuple(args, "idOO", &iter, &epsilon, &py_vectors, &py_centroids))
        return NULL;

    struct vector *vectors = vectors_from_py_list_list(py_vectors);
    struct vector *centroids = vectors_from_py_list_list(py_centroids);

    if (vectors == NULL || centroids == NULL) {
        // Some error in the conversion
        return NULL;
    }

    int N = PyList_Size(py_vectors);
    int K = PyList_Size(py_centroids);

    struct vector *resulting_centroids = kmeans(N, K, iter, vectors, centroids, epsilon);
    PyObject *py_centroids_result = list_list_float_from_vectors(resulting_centroids);

    free_vector(vectors);
    free_vector(resulting_centroids);

    return py_centroids_result;
}
