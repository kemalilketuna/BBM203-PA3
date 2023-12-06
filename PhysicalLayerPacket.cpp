#include "PhysicalLayerPacket.h"

PhysicalLayerPacket::PhysicalLayerPacket(int _layer_ID, const string& _sender_MAC, const string& _receiver_MAC)
        : Packet(_layer_ID) {
    sender_MAC_address = _sender_MAC;
    receiver_MAC_address = _receiver_MAC;
}

void PhysicalLayerPacket::print() {
    cout << "Sender MAC address: " << sender_MAC_address << ", Receiver MAC address: " << receiver_MAC_address << endl;
}

PhysicalLayerPacket::~PhysicalLayerPacket() {
    // TODO: Free any dynamically allocated memory if necessary.
}

void PhysicalLayerPacket::increase_hop_count() {
    hop_count++;
}

int PhysicalLayerPacket::get_hop_count() {
    return hop_count;
}

void PhysicalLayerPacket::set_frame_idx(int _frame_idx) {
    if(frame_idx == 0 && _frame_idx > 0)
        frame_idx = _frame_idx;
}

int PhysicalLayerPacket::get_frame_idx() {
    return frame_idx;
}