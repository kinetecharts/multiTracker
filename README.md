Weidong Yang
2014-05-27
Tracking objects and stream data via osc

Frame rate set to 30fps. On my 2012MBP, cpu drop from 40% to 34%. Reducing frame rate to 15fps further drops cpu to 30%. I did some profiling, blob detection and contour finding uses just couple % of cpu. It seems majority cpu time is consumed in communication with PS3 eye. 

This is true even with build in isight camera. It seems getting data from camera is the most time consuming part.

Lower resolution by half (640 => 320) drops cpu from 40% to 10%. 

Using the app:
1. check Auto Gain and Shutter
2. uncheck once exposure is correct. (alternatively, play with gain and shutter, try set shutter at 1 to reduce light flicker)
3. click "space"  to set background
4. Check Send Centers will stream center location of contours (/cur label x y)
5. Check Send Target Details will send contour bounding box 
	(/contour label size TLx TLy BRx BRy) size is num points in polyline
6. Check Send Contours (only effect when Send Target Details is checked) streams full contour
	(/contour label size TLx TLy BRx BRy P0x P0y P1x P1y .... )
	Some time contour may have so many points that it cause osc out of memory. Need to simplify polyline.
7. Skip Sample will cause app to skip number of frames before send out next osc data. Purpose is to lower the traffic.
8. Gain
9. Shutter. Try to set to 1 or close to 1 to reduce light flicker
10. Blob Min Radius: minimum radius of a blob can be accepted
11 Blob Max Radius
12 Learning time. Controls how fast background is refreshed. A close to 0 number will cause it to refresh almost real time. When set to 1000, background almost never shift.
