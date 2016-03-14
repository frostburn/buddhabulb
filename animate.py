import subprocess

width = 300
height = 300
layer = 0
num_layers = 50
num_frames = 60

normalizer = 140000

subprocess.call(["rm", "tmp/*.rgb"])
# subprocess.call(["rm", "./tmp/*.png"])

for i in range(num_frames):
    subprocess.call(map(str, ["./process", width, height, layer, num_layers, i, num_frames, normalizer]))
    subprocess.call(["mv", "out.raw", "tmp/out%02d.rgb" % (i + 1)])

subprocess.call(["convert", "-delay", "10", "-loop", "0", "-depth", "8", "-size", "%dx%d" % (width, height), "./tmp/out*.rgb", "out.gif"])

# subprocess.call(["mogrify", "-delay", "10", "-loop", "0", "-depth", "8", "-size", "%dx%d" % (width, height), "-format", "png", "./tmp/out*.rgb"])
