#!/usr/bin/python

##
# @file fp_io.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  I/O functionality for .fp files
#
# @section DESCRIPTION
#
# This file contains functions that will parse floorplans from
# .fp files and allow for various processing on them.
#

##
# The Floorplan class holds information from .fp files
#
class Floorplan:
    'Class defines floorplan geometry information'

    ##
    # Constructor generates class from .fp file
    #
    # This class has the following fields:
    #
    #    res -        resolution of floorplan (meters)
    #    num_verts -    number of vertices
    #    num_tris -    number of triangles
    #    num_rooms -     number of rooms
    #    verts -        Array of vertices, each vert a tuple (x,y)
    #    tris -        Array of triangles, each tri a tuple (i,j,k)
    #    room_tris -    Array of room triangle indices, each element
    #                   is an array of triangle indices for given room
    #    room_min_z -    Array of room floor heights (meters)
    #    room_max_z -    Array of room floor heights (meters)
    #
    def __init__(self, input_file=None):

        # check arguments
        if input_file is None:
            # prepare an empty structure
            self.res = -1.0 # invalid resolution
            self.num_verts = 0
            self.num_tris = 0
            self.num_rooms = 0
            self.verts = []
            self.tris = []
            self.room_tris = []
            self.room_min_z = []
            self.room_max_z = []
        else:
            # read structure from file
            self.read(input_file)

    ##
    # Reads floorplan data from the specified .fp file
    #
    # Will parse the file specified as a .fp file
    #
    # @param input_file    The path to the input .fp file to parse
    #
    def read(self, input_file):

        # attempt to open file to read
        with open(input_file) as f:
            # read file
            content = f.read().splitlines()
        
        # check formatting
        if len(content) < 4:
            raise IOError("Invalid header in .fp file:" + \
                    input_file)

        # store header in object
        self.res = float(content[0]) # resolution (units: meters)
        self.num_verts = int(content[1]) # number of vertices
        self.num_tris = int(content[2]) # number of triangles
        self.num_rooms = int(content[3]) # number of rooms

        # check if contents match the header
        if len(content) < (4 + self.num_verts \
                + self.num_tris + self.num_rooms):
            raise IOError("Invalid formatting in .fp file: " + \
                    input_file)

        # iterate over vertices in file
        self.verts = []
        offset = 4
        for i in range(self.num_verts):

            # get the position of the i'th vertex
            v = content[offset+i].split()
        
            # check for correct number of dimensions
            if len(v) != 2:
                raise IOError("Bad .fp file: Vertex #" \
                        + str(i) \
                        + " not valid: " \
                        + str(v))

            # store in structure as tuple (x,y)
            self.verts.append( (float(v[0]), float(v[1])) )

        # iterate over triangles in file
        self.tris = []
        offset += self.num_verts
        for i in range(self.num_tris):

            # get the values for the i'th triangle
            t = content[offset+i].split()

            # check valid formatting
            if len(t) != 3:
                raise IOError("Bad .fp file: Triangle #" \
                        + str(i) \
                        + " not valid: " \
                        + str(t))

            # store structure in file as tuple of three indices
            # to vertices in counter-clockwise order
            self.tris.append( (int(t[0]),int(t[1]),int(t[2])) )

        # iterate over rooms in file
        self.room_tris  = [] # triangle indices for each room
        self.room_min_z = [] # floor heights for each room
        self.room_max_z = [] # ceiling heights for each room
        offset += self.num_tris
        for i in range(self.num_rooms):

            # get content for i'th room
            r = content[offset+i].split()

            # check validity
            if len(r) < 3:
                raise IOError("Bad .fp file: Room #" \
                        + str(i) \
                        + " not valid: " \
                        + str(r))

            # get room-level info
            self.room_min_z = float(r[0])
            self.room_max_z = float(r[1])
            num_tris = int(r[2])

            # check validity of the remainder of the line
            if len(r) != (3+num_tris):
                raise IOError("Bad .fp file: Room #" \
                        + str(i) \
                        + " had incorrect " \
                        + "number of triangles: " \
                        + str(r))
            
            # get triangle indices for this room
            tri_inds = []
            for j in range(num_tris):
                tri_inds.append(int(r[3+j]))
            self.room_tris.append(tri_inds)

    ##
    # Computes bounds of the loaded floorplan
    #
    # Will compute the ((x_min, x_max), (y_min, y_max))
    # bounds of the floorplan represented by this object.
    #
    # @return  Returns ((x_min, x_max), (y_min, y_max))
    #
    def compute_bounds(self):
        
        # initialize bounds
        x_min = 0
        x_max = 0
        y_min = 0
        y_max = 0

        # iterate over vertices
        for v in self.verts:

            # compare current vertex to bounds
            (x, y) = v
            if x < x_min:
                x_min = x
            if x > x_max:
                x_max = x
            if y < y_min:
                y_min = y
            if y > y_max:
                y_max = y

        # return the computed bounds
        return ((x_min, x_max), (y_min, y_max))

    ##
    # Computes boundary edges of this floorplan
    #
    # A boundary edge is a triangle edge that represents a 
    # in the floorplan
    #
    # @return  Returns a list of tuples, each an edge as vertex indices
    #
    def compute_boundary_edges(self):
        return self.compute_boundary_edges_for(range(len(self.tris)))
    
    ##
    # Computes boundary edges of the specified room
    #
    # A boundary edge is between two vertices that share exactly
    # one triangle.  This means that the edge is not interior
    # to the floor plan.
    #
    # @param roomid     The index of the room to generate edges for
    #
    # @return  Returns a list of tuples, each an edge as vertex indices
    #
    def compute_room_boundary_edges(self, roomid):
        return self.compute_boundary_edges_for(self.room_tris[roomid])

    ##
    # Computes boundary edges for a subset of triangles in this floorplan
    #
    # Given a list of triangles, will compute all edges that occur
    # in exactly one triangle in the list without the reverse edge appearing
    # which denotes a boundary edge.
    #
    # @param tri_inds   The list of indices of triangles to use
    #
    # @return   Returns a list of tuples, each an edge as vertex indices
    def compute_boundary_edges_for(self, tri_inds):

        # put edges into a list
        all_edges = []
        for ti in tri_inds:
            # get the triangle
            (i,j,k) = self.tris[ti]
            
            # record all edges
            all_edges.append((i,j))
            all_edges.append((j,k))
            all_edges.append((k,i))
            
        boundary_edges = []
        for (i,j) in all_edges:
            # check if the opposing edge exists
            if (j,i) not in all_edges:
                boundary_edges.append((i,j))

        # return boundary edges
        return boundary_edges

    ##
    # Compute the oriented boundary edges of floorplan
    #
    # Will list all boundary vertices of the given set
    # of triangles in this floorplan, in counter-clockwise
    # order.  Disjoint boundary sets will be placed in their
    # own lists.
    #
    # Exports a list of lists.  Each sublist corresponds to
    # a connected boundary in the floorplan.
    #
    # @return  Returns list of boundaries
    #
    def compute_oriented_boundary(self):
        return self.compute_oriented_boundary_edges_for( \
                                range(len(self.tris)))

    ##
    # Compute the oriented boundary edges for a single room in the floorplan
    #
    # Will list all boundary vertices of the set of triangles in the given
    # room, in counter-clockwise order.  Disjoint boundary sets will be
    # placed in their own lists.
    #
    # Exports a list of lists.  Each sublist corresponds to a connected
    # boundary in the room.
    #
    # @return   Returns list of boundaries
    #
    def compute_room_oriented_boundary(self, roomid):
        return self.compute_oriented_boundary_for(self.room_tris[roomid])


    ##
    # Compute the oriented boundary edges for a subset of triangles
    #   
    # Given a subset of triangles from this floorplan, will compute
    # the oriented set of boundary edges.  Will list edges in counter-
    # clockwise order.  Disjoint boundary sets will be placed in their
    # own lists.
    #
    # Exports a list of lists.  Each sublist corresponds to a connected
    # boundary in the floorplan.
    #
    # @return   Returns list of boundaries
    def compute_oriented_boundary_for(self, tri_inds):

        # compute all boundary edges
        all_edges = self.compute_boundary_edges_for(tri_inds)

        # generate a mapping between starting vertex and edges
        edge_map = {}
        for (i,j) in all_edges:
            if i in edge_map:
                edge_map[i].append((i,j))
            else:
                edge_map[i] = [(i,j)]

        # initialize output metalist
        boundary_list = []

        # build up each disjoint boundary
        while len(all_edges) > 0:

            # start a new boundary
            boundary_list.append([])

            # get the next edge, and follow it
            e = all_edges.pop()
            boundary_list[-1].append(e[0])
            boundary_list[-1].append(e[1])
            last = e[1]
            edge_map[e[0]].remove(e)

            # follow this edge as long as possible to
            # form an oriented boundary
            while len(edge_map[last]) > 0:
                
                # get next edge from the last observed vertex
                e = edge_map[last].pop()
                all_edges.remove(e)

                # check if we've made a loop
                last = e[1]
                if last == boundary_list[-1][0]:
                    # this is the vertex where we started, loop complete
                    break

                # follow this edge
                boundary_list[-1].append(last)

        # we've populated all boundaries
        return boundary_list 
