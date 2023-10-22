# ImageProcessing
This is an Image processing application made in c++. It supports the following features:

● `i <filename> as <$token>`. Import an image file named filename from
the filesystem, which corresponds to the unique identifier $token.

● e <$token> as <filename>. Export the image associated with the 
$token to a file with path filename. If the image is black and white it is exported in PGM format,
while if the image is in color it is exported in PPM format.

●   d <$token>. Deletes the unique identifier $token from memory along with the image corresponding to it.

●   n <$token>. Reverses the brightness of the corresponding image to the unique identifier $token.

●   z <$token>. Histogram equalization to the image is corresponding to the unique identifier $token is performed.

●   m <$token>. The image corresponding to the unique identifier $token is reversed (mirror) along its vertical axis.

●   g <$token>. If the image is black and white, no action is taken. If it is in color, then the original image is 
replaced by the corresponding black and white, which binds to the same "$token" id. The original color image is deleted.

●   s <$token> by <factor>. The image corresponding to the unique identifier "$token" is scaled the "factor" floating point number.

●   r <$token> clockwise <Χ> times. The image corresponding to the unique id $token rotates clockwise as many times as
describes the integer parameter X. If X is negative number the image rotates counterclockwise as many 
times as the absolute describesvalue of X.

●   q. Terminates the program. Before termination all the memory that was previously
committed is freed.
