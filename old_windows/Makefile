# 2024-06-13 Keith D Eilor

LIBS = -L/usr/local/lib -lopencv_gapi -lopencv_highgui -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_video -lopencv_calib3d -lopencv_features2d -lopencv_dnn -lopencv_flann -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_core

CFLAGS = -I/usr/local/include/opencv4

SMASSystem:  SMASSystem.cpp
	g++ $(CFLAGS) $(LIBS) -o $@ $<

clean:
	rm SMASSystem


