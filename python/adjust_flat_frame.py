import cv2 as cv
from sys import argv

if __name__ == '__main__':
    if len(argv) < 3:
        print("Usage: python adjust_flat_frame.py <input_file> <output_file> <scale_factor>")
        print("The last argument is optional and defaults to 3")
        exit(1)
    input_file = argv[1]
    output_file = argv[2]
    scale_factor = float(argv[3]) if len(argv) > 3 else 3.0

    # Read the input image
    image = cv.imread(input_file, cv.IMREAD_UNCHANGED)

    print("Input image:")
    print(image.shape)
    print(image.dtype)
    print(image.max())
    print(image.min())

    image_as_floats = image.astype(float)
    max_value = image_as_floats.max()
    adjusted_image = (image_as_floats + (scale_factor * max_value)) / (1 + scale_factor)

    # Clip the values to the valid range [0, 65535]
    adjusted_image = adjusted_image.clip(0, 65535)

    # Convert back to uint16
    adjusted_image = adjusted_image.astype('uint16')

    #print(adjusted_image)
    print("Output image:")
    print(adjusted_image.shape)
    print(adjusted_image.dtype)
    print(adjusted_image.max())
    print(adjusted_image.min())

    # Save the output image

    cv.imwrite(output_file, adjusted_image)
