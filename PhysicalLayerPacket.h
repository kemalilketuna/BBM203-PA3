#ifndef PHYSICAL_LAYER_PACKET_H
#define PHYSICAL_LAYER_PACKET_H

#include "Packet.h"

using namespace std;

// Extends Packet class. Have additional layer-specific member variables and overrides the virtual print function.
class PhysicalLayerPacket : public Packet {
public:
    PhysicalLayerPacket(int layerID, const string& senderMAC, const string& receiverMAC);
    ~PhysicalLayerPacket() override;

    string sender_MAC_address;
    string receiver_MAC_address;
    void increase_hop_count();
    int get_hop_count();
    void set_frame_idx(int chunk_count);
    int get_frame_idx();
    void print() override;
private:
    int hop_count = 0;
    int frame_idx = 0;
};

#endif // PHYSICAL_LAYER_PACKET_H
