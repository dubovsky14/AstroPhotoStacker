from sys import argv
import cv2


if __name__ == "__main__":

    # Check if the correct number of arguments is provided
    if len(argv) != 5:
        print("Usage: python shorten_video.py <input_video> <output_video> <n_frames> <fps_new>")
        print("Example: python shorten_video.py input.mp4 output.mp4 100 30.0")
        exit(1)

    video_file_address = argv[1]
    output_file_address = argv[2]
    n_frames = int(argv[3])
    fps_new = float(argv[4])

    # Open the input video file
    cap = cv2.VideoCapture(video_file_address)
    if not cap.isOpened():
        print(f"Error: Could not open video file {video_file_address}")
        exit(1)

    input_codec = int(cap.get(cv2.CAP_PROP_FOURCC))

    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

    # Define the codec and create VideoWriter object
    output_video = cv2.VideoWriter(output_file_address, input_codec, fps_new, (width, height))
    if (output_video.isOpened() == False):
        print("Error opening output stream or file")

    for i_frame in range(n_frames):
        # Read the next frame
        ret, frame = cap.read()
        if not ret:
            print(f"Error: Could not read frame {i_frame}")
            break
        # Write the frame to the output video
        output_video.write(frame)
    # Release the video objects
    cap.release()
    output_video.release()
