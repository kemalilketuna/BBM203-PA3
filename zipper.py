import zipfile

# List of files to be zipped
files = [
    "ApplicationLayerPacket.cpp",
    "ApplicationLayerPacket.h",
    "Client.cpp",
    "Client.h",
    "Log.cpp",
    "Log.h",
    "Network.cpp",
    "Network.h",
    "NetworkLayerPacket.cpp",
    "NetworkLayerPacket.h",
    "Packet.cpp",
    "Packet.h",
    "PhysicalLayerPacket.cpp",
    "PhysicalLayerPacket.h",
    "TransportLayerPacket.cpp",
    "TransportLayerPacket.h"
]

# Create a new zip file (or overwrite if it exists)
with zipfile.ZipFile("b2220356127.zip", "w") as z:
    for file in files:
        z.write(file)

print("Files zipped successfully into output.zip")