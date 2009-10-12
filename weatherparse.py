#************************************************************************#
#*									                                    *#
#*		             P R O J E K T    A V A L O N 		                *#
#*								                                    	*#
#*	    weatherparse.py     Script for parsing pngs into weather info.	*#
#*								                                    	*#
#*	    Author          	Stefan Wismer		                        *#
#*                          wismerst@student.ethz.ch                    *#
#*									                                    *#
#************************************************************************#

# Image-Libary
import gd

# DDX Access
import ddxInterface

# Init DDX Interface
store = ddxInterface.ddxStore()

# Get handle to the weather variable
V = store.variable('weather')

V.timestamp = 200903041200

# Load Images and read size
image_u = gd.image("testfiles/wind_u.png");
image_v = gd.image("testfiles/wind_v.png");

width=image_u.size()[0]
height=image_u.size()[1]

print "Die Bilder sind", width, "breit und", height, "hoch"

# max wind is assumed to be known for now... (random values)
u_max = 19.33
v_max = 20.81

# Then start reading the pixels
i = 0
j = 0

# for debugging (screen is not so filled....)
# width = 20
# height = 20

while i < height:
    while j < width:

        pixu = (image_u.getPixel((j+1,80-i))-115.55) / 7.24;
        pixv = (image_v.getPixel((j+1,80-i))-115.55) / 7.24;

        V.xdata[j][i] = pixu
        V.ydata[j][i] = pixv
        print "(%4.1f, %4.1f) " % (pixu,pixv),
        j = j + 1
    print '\n',
    i = i + 1
    j = 0

V.write()
print "Fertig."
