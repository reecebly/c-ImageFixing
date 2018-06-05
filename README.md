# c-ImageFixing
Remove scratch from image and create a new image that has nearly the same quality as the original

Take in original image and search through the 3 color channels until we identify the correct channel
We then find the threshold that has the entire scratch in it and extract the detail
From here we separate the scratch from the rest of the detail
Add back the other details to the blurred image to create a restored image
