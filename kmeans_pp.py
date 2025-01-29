import sys
import math
from dataclasses import dataclass
import pandas as pd
import os
import numpy as np


@dataclass
class arguments:
    k: int
    iter: int
    epsilon: float
    input_file_name_1: str
    input_file_name_2: str


def parse_args(
    k: int, iter: int, epsilon: float, input_file_1: str, input_file_2: str
) -> np.ndarray:
    if not (1 < iter < 1000):
        print("Invalid maximum iteration!")
        exit()

    if epsilon < 0:
        print("Invalid epsilon")
        exit()

    file_name_1, file_extension_1 = os.path.splitext(input_file_1)
    file_name_2, file_extension_2 = os.path.splitext(input_file_2)
    is_text_1 = False
    is_text_2 = False

    if file_extension_1 == "txt":
        is_text_1 = True

    if file_extension_2 == "txt":
        is_text_2 = True

    df1 = pd.read_csv(input_file_1, delim_whitespace=is_text_1, header=None)
    df2 = pd.read_csv(input_file_2, delim_whitespace=is_text_2, header=None)

    # id_column_1 = df1.columns[0]
    # id_column_2 = df2.columns[0]
    # df1 = df1.drop(columns=[id_column_1])
    # df2 = df2.drop(columns=[id_column_2])

    df1.rename(columns={df1.columns[0]: "key column"}, inplace=True)
    df2.rename(columns={df2.columns[0]: "key column"}, inplace=True)

    points_df = pd.merge(df1, df2, on="key column")
    points_df.rename(columns={points_df.columns[0]: "key column"}, inplace=True)
    points_df.sort_values(by="key column", ascending=True, inplace=True)
    points_df.drop("key column", axis=1, inplace=True)

    if not (1 < k < len(points_df)):
        print("Invalid number of clusters!")
        exit()

    return points_df.to_numpy()


def print_centroids(centroids: list[list[float]]):
    for centroid in centroids:
        centroid_string = ",".join(["{0:.4f}".format(cord) for cord in centroid])
        print(centroid_string)


def get_arguments(argv: list):
    k = int(argv[1])
    if len(argv) == 6:
        iter = int(argv[2])
        i = 3
    else:
        iter = 300
        i = 2

    epsilon = float(argv[i])
    input_file_name_1 = argv[i + 1]
    input_file_name_2 = argv[i + 2]

    return arguments(k, iter, epsilon, input_file_name_1, input_file_name_2)


def compute_Dx_array(points: np.ndarray, centroids: np.ndarray) -> np.ndarray:
    distances = np.linalg.norm(points[:, np.newaxis] - centroids, axis=2)
    return np.min(distances, axis=1)


def compute_Px(dx: np.ndarray) -> np.ndarray:
    total = np.sum(dx)
    return dx / total


def centroid_initialization(k: int, points_np_array: np.ndarray):
    np.random.seed(1234)
    first_centroid_index = np.random.choice(points_np_array.shape[0])
    first_centroid = points_np_array[first_centroid_index]
    centroids_indexes = [first_centroid_index]
    centroids = np.array([first_centroid])

    print(centroids)
    for _ in range(k - 1):
        Dx = compute_Dx_array(points_np_array, centroids)
        Px = compute_Px(Dx)
        next_centroid_index = np.random.choice(points_np_array.shape[0], p=Px)
        next_centroid = points_np_array[next_centroid_index]
        centroids = np.append(centroids, [next_centroid], axis=0)

        centroids_indexes.append(next_centroid_index)

    return centroids, centroids_indexes


def main():
    args: arguments = get_arguments(sys.argv)
    points_np_array = parse_args(
        args.k, args.iter, args.epsilon, args.input_file_name_1, args.input_file_name_2
    )
    centroids, centroids_indexes = centroid_initialization(args.k, points_np_array)
    print(centroids_indexes)

    print_centroids(centroids)


if __name__ == "__main__":
    main()
