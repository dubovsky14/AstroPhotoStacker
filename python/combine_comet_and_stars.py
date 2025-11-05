import sys
import numpy as np
import cv2 as cv

def combine_images(comet_stack, comet_rms, stars_stack, stars_rms):
    # Ensure all images have the same shape
    assert comet_stack.shape == stars_stack.shape, "Comet and stars image stacks must have the same shape"
    assert comet_rms.shape == stars_rms.shape, "Comet and stars RMS images must have the same shape"
    assert comet_stack.shape == comet_rms.shape, "Comet image stack and RMS must have the same shape"

    combined_stack = np.zeros_like(comet_stack)

    relative_rms_comet = comet_rms / (comet_stack + 1e-6)
    relative_rms_stars = stars_rms / (stars_stack + 1e-6)

    mask_comet_better = relative_rms_comet < relative_rms_stars
    #mask_comet_better = comet_rms < stars_rms
    combined_stack = comet_stack*mask_comet_better + stars_stack*(~mask_comet_better)

    print("Maximum RMS in comet image: {:.6f}".format(np.max(comet_rms)))
    print("Average RMS in comet image: {:.6f}".format(np.mean(comet_rms)))

    print("Maximum RMS in stars image: {:.6f}".format(np.max(stars_rms)))
    print("Average RMS in stars image: {:.6f}".format(np.mean(stars_rms)))

    print("Percentage of pixels taken from comet image: {:.2f}%".format(100.0 * np.sum(mask_comet_better) / mask_comet_better.size))

    return combined_stack

if __name__ == "__main__":
    if len(sys.argv) != 6:
        print("Usage: python combine_comet_and_stars.py <comet_image_stack> <comet_image_rms> <stars_image_stack> <stars_image_rms> <output_image>")
        sys.exit(1)

    comet_image_stack   = cv.imread(sys.argv[1], cv.IMREAD_UNCHANGED).astype(np.float32)
    comet_image_rms     = cv.imread(sys.argv[2], cv.IMREAD_UNCHANGED).astype(np.float32)
    stars_image_stack   = cv.imread(sys.argv[3], cv.IMREAD_UNCHANGED).astype(np.float32)
    stars_image_rms     = cv.imread(sys.argv[4], cv.IMREAD_UNCHANGED).astype(np.float32)
    output_image_path   = sys.argv[5]

    combined_image = combine_images(comet_image_stack, comet_image_rms, stars_image_stack, stars_image_rms)
    cv.imwrite(output_image_path, combined_image.astype(np.uint16))