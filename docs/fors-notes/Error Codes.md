Many functions in the kernel return an integer status code. A value of 0 means that the operation was successful, and a negative value represents that some error occurred. Some error codes are general, and some are specific to certain functions. The general ones are:

 - Generic error (-1)
 - Invalid argument (-2)
 - Functionality not implemented (-3)
 - Operation not permitted (-4)