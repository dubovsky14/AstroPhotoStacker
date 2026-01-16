from sys import argv
import cv2

if __name__ == '__main__':
    if len(argv) < 5:
        print("Usage:  python resize_flat_frame.py <input_image> <output_image> <width>x<height> <crop factor> <rotation in degrees (default is 0)>")
        exit(1)

    input_image = argv[1]
    output_image = argv[2]
    width, height = map(int, argv[3].split('x'))
    crop_factor = float(argv[4])
    rotation_degrees = float(argv[5]) if len(argv) > 5 else 0

    image = cv2.imread(input_image, cv2.IMREAD_UNCHANGED)
    if image is None:
        print(f"Error: Unable to load image {input_image}")
        exit(1)

    (original_h, original_w) = image.shape[:2]
    if rotation_degrees != 0:
        (h, w) = image.shape[:2]
        center = (w // 2, h // 2)
        M = cv2.getRotationMatrix2D(center, rotation_degrees, 1.0)
        image = cv2.warpAffine(image, M, (w, h))

    if crop_factor != 1.0:
        h, w = image.shape[:2]
        new_w, new_h = int(original_w / crop_factor), int(original_h / crop_factor)
        start_x = (w - new_w) // 2
        start_y = (h - new_h) // 2
        image = image[start_y:start_y + new_h, start_x:start_x + new_w]

    resized_image = cv2.resize(image, (width, height), interpolation=cv2.INTER_AREA)
    cv2.imwrite(output_image, resized_image)
