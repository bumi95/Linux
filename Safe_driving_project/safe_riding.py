import face_recognition
import cv2
import numpy as np
import zmq

# This is a demo of running face recognition on live video from your webcam. It's a little more complicated than the
# other example, but it includes some basic performance tweaks to make things run a lot faster:
#   1. Process each video frame at 1/4 resolution (though still display it at full resolution)
#   2. Only detect faces in every other frame of video.

# PLEASE NOTE: This example requires OpenCV (the `cv2` library) to be installed only to read from your webcam.
# OpenCV is *not* required to use the face_recognition library. It's only required if you want to run this
# specific demo. If you have trouble installing it, try any of the other demos that don't require it instead.
def gstreamer_pipeline(
    capture_width=3280,
    capture_height=2464,
    display_width=960,
    display_height=720,
    framerate=21,
    flip_method=0,
    ):
    return (
        "nvarguscamerasrc ! "
        "video/x-raw(memory:NVMM), "
        "width=(int)%d, height=(int)%d, "
        "format=(string)NV12, framerate=(fraction)%d/1 ! "
        "nvvidconv flip-method=%d ! "
        "video/x-raw, width=(int)%d, height=(int)%d, format=(string)BGRx ! "
        "videoconvert ! "
        "video/x-raw, format=(string)BGR ! appsink"
        % (
            capture_width,
            capture_height,
            framerate,
            flip_method,
            display_width,
            display_height,
        )
    )


# Get a reference to webcam #0 (the default one)
video_capture = cv2.VideoCapture(gstreamer_pipeline(flip_method=0), cv2.CAP_GSTREAMER)

n_image = face_recognition.load_image_file("normal1.jpg")
n_face_encoding = face_recognition.face_encodings(n_image)[0]

n2_image = face_recognition.load_image_file("normal2.jpg")
n2_face_encoding = face_recognition.face_encodings(n2_image)[0]

un_image = face_recognition.load_image_file("abnormal1.jpg")
un_face_encoding = face_recognition.face_encodings(un_image)[0]

un2_image = face_recognition.load_image_file("abnormal2.jpg")
un2_face_encoding = face_recognition.face_encodings(un2_image)[0]
known_face_encodings = [
    n_face_encoding,
    n2_face_encoding,
    un_face_encoding,
    un2_face_encoding
]
known_face_names = [
    "safe",
    "safe",
    "dangerous",
    "dangerous"
]
# Initialize some variables
face_locations = []
face_encodings = []
face_names = []
process_this_frame = 0

while True:
    # Grab a single frame of video

    ret, frame = video_capture.read()
    # Resize frame of video to 1/4 size for faster face recognition processing
    small_frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)

    # Convert the image from BGR color (which OpenCV uses) to RGB color (which face_recognition uses)
    rgb_small_frame = small_frame[:, :, ::-1]

    # Only process every other frame of video to save time
    if process_this_frame == 3:
        # Find all the faces and face encodings in the current frame of video
        face_locations = face_recognition.face_locations(rgb_small_frame)
        face_encodings = face_recognition.face_encodings(rgb_small_frame, face_locations)
        process_this_frame = 0

        face_names = []
        for face_encoding in face_encodings:
            # See if the face is a match for the known face(s)
            matches = face_recognition.compare_faces(known_face_encodings, face_encoding)
            name = "dangerous"
    
            # # If a match was found in known_face_encodings, just use the first one.
            # if True in matches:
            #     first_match_index = matches.index(True)
            #     name = known_face_names[first_match_index]

            # Or instead, use the known face with the smallest distance to the new face
            face_distances = face_recognition.face_distance(known_face_encodings, face_encoding)
            best_match_index = np.argmin(face_distances)
            if matches[best_match_index]:
                name = known_face_names[best_match_index]

            face_names.append(name)

        context = zmq.Context()

        socket = context.socket(zmq.REQ)
        socket.connect("tcp://localhost:5555")

        socket.send(b"velocity")

        message = socket.recv()

        decode_msg = message.decode("utf-8")
        cv2.putText(frame, decode_msg, (0, 100), cv2.FONT_HERSHEY_SCRIPT_SIMPLEX, 3, (0, 0, 255))

    process_this_frame+=1

    # Display the results
    for (top, right, bottom, left), name in zip(face_locations, face_names):
        # Scale back up face locations since the frame we detected in was scaled to 1/4 size
        top *= 4
        right *= 4
        bottom *= 4
        left *= 4

        # Draw a box around the face
        if name == "safe" :
            cv2.rectangle(frame, (left, top), (right, bottom), (0, 255, 0), 2)
            cv2.rectangle(frame, (left, bottom - 35), (right, bottom), (0, 255, 0), cv2.FILLED)
        
        else :
            cv2.rectangle(frame, (left, top), (right, bottom), (0, 0, 255), 2)
            cv2.rectangle(frame, (left, bottom - 35), (right, bottom), (0, 0, 255), cv2.FILLED)
        font = cv2.FONT_HERSHEY_DUPLEX
        cv2.putText(frame, name, (left + 6, bottom - 6), font, 1.0, (255, 255, 255), 1)
    #cv2.putText(frame, decode_msg, (0, 100), cv2.FONT_HERSHEY_SCRIPT_SIMPLEX, 3, (0, 0, 255))
    # Display the resulting image
    cv2.imshow('Video', frame)


    # Hit 'q' on the keyboard to quit!
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break


# Release handle to the webcam
video_capture.release()
cv2.destroyAllWindows()
