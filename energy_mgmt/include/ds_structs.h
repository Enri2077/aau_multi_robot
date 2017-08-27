#ifndef DS_STRUCTS_H
#define DS_STRUCTS_H

struct ds_t
{
    int id;
    double x, y; // coordinates of the DS in the /map frame
    double world_x, world_y; // coordinates of the DS in the /world frame (i.e., in case of a simulation, in the reference system of the simulator)
    bool vacant;
    double timestamp;
    bool has_EOs;
};

#endif // DS_STRUCTS_H