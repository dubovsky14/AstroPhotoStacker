from sys import argv

import numpy as np

def rotate_star_coordinates(star_coordinates : list[np.ndarray], angle : float) -> list[np.ndarray]:
    rotation_matrix = np.array([[np.cos(angle), -np.sin(angle)], [np.sin(angle), np.cos(angle)]])
    rotated_coordinates = []
    for coord in star_coordinates:
        rotated_coord = np.dot(rotation_matrix, coord)
        rotated_coordinates.append(rotated_coord)
    return rotated_coordinates

def scale_star_coordinates(star_coordinates : list[np.ndarray], scale_factor : float) -> list[np.ndarray]:
    scaled_coordinates = []
    for coord in star_coordinates:
        scaled_coord = coord * scale_factor
        scaled_coordinates.append(scaled_coord)
    return scaled_coordinates

def translate_star_coordinates(star_coordinates : list[np.ndarray], translation_vector : np.ndarray) -> list[np.ndarray]:
    translated_coordinates = []
    for coord in star_coordinates:
        translated_coord = coord + translation_vector
        translated_coordinates.append(translated_coord)
    return translated_coordinates

def get_normalized_stars_coordinates(asterism_hash : list[float]) -> list[np.ndarray]:
    if len(asterism_hash) != 4:
        raise ValueError("Expected 4 coordinates in asterism_hash")
    result = [np.array([0, 0]), np.array([1, 1]), np.array([asterism_hash[0], asterism_hash[1]]), np.array([asterism_hash[2], asterism_hash[3]])]
    return result

if __name__ == "__main__":
    asterism_hash = [0.23, 0.45, 0.67, 0.89]
    angle = (np.pi / 180) * 67
    translation = np.array([1.6,4.])

    print("Asterism hash:", asterism_hash)
    star_coordinates = get_normalized_stars_coordinates(asterism_hash)
    print("Original star coordinates:")
    for coord in star_coordinates:
        print(coord)

    rotated_coordinates = rotate_star_coordinates(star_coordinates, angle)
    print("\nRotated star coordinates:")
    for coord in rotated_coordinates:
        print(coord)

    scaled_coordinates = scale_star_coordinates(rotated_coordinates, 2.0)
    print("\nScaled star coordinates:")
    for coord in scaled_coordinates:
        print(coord)

    translated_coordinates = translate_star_coordinates(scaled_coordinates, translation)
    print("\nTranslated star coordinates:")
    for coord in translated_coordinates:
        print("{" + str(round(coord[0],5)) + ", " + str(round(coord[1],5)) + ", 1},")

