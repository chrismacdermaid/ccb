// -*-c++-*-

// *hd +------------------------------------------------------------------------------------+
// *hd |  This file is part of Coiled-Coil Builder.                                         |
// *hd |                                                                                    |
// *hd |  Coiled-Coil Builder is free software: you can redistribute it and/or modify       |
// *hd |  it under the terms of the GNU General Public License as published by              |
// *hd |  the Free Software Foundation, either version 3 of the License, or                 |
// *hd |  (at your option) any later version.                                               |
// *hd |                                                                                    |
// *hd |  Coiled-Coil Builder is distributed in the hope that it will be useful,            |
// *hd |  but WITHOUT ANY WARRANTY without even the implied warranty of                     |
// *hd |  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                     |
// *hd |  GNU General Public License for more details.                                      |
// *hd |                                                                                    |
// *hd |  You should have received a copy of the GNU General Public License                 |
// *hd |  along with Coiled-Coil Builder.  If not, see <http:www.gnu.org/licenses/>.        |
// *hd +------------------------------------------------------------------------------------+

// *hd | If you intend to use this software for your research, please cite:
// *hd | and inform Chris MacDermaid <chris.macdermaid@gmail.com> of any pending publications.

// *hd | Copyright (c) 2012,2013,2014 by Chris M. MacDermaid <chris.macdermaid@gmail.com>
// *hd | and Jeffery G. Saven <saven@sas.upenn.edu>


/**
 * @file   site.cpp
 * @author Chris Von Bargen <cvb@magrathea.chem.upenn.edu>
 * @date   Nov 14 2011
 *
 * @brief  Site, describing a location in the domain
 *
 * Class to describe a specific site in the domain.
 * Here contains descriptive fields (name, resid, etc.) as well as
 * a static group of fixed common atoms and a collection of corresponding groups (rotamers)
 *
 */

#include "string.h"
#include "memory.h"
#include "error.h"
#include "domain.h"
#include "site.h"
#include "group.h"
#include "atom.h"
#include "ccbio.h"
#include "sort.h"
#include "universe.h"

/**
 * @def ROTAMER_DELTA
 *
 * @brief Number of rotamers to reallocate each time we run out of space
 */
#define ROTAMER_DELTA 50

/**
 * @def TYPES_PER_GROUP
 * @brief Maximum number of types per group
 *
 */
#define TYPES_PER_GROUP 50

using namespace CCB_NS;

Site::Site(CCB *ccb, int myid) :
        Pointers(ccb),
        id(myid),
        resid(0),
        fixed_atoms(),
        rotamer(),
        most_probable(),
        nrotamer(0),
        mask(1),
        ngroup_iter(0),
        maxrotamer(0)
{

    // Initialize variables
    chain[0] = '\0';
    seg[0] = '\0';

    fixed_atoms = new Group(ccb, -1);
    fixed_atoms->site = this;
}

/**
 * Domain Deconstructor
 *
 * Delete fixed_atoms and all rotamers
 */
Site::~Site() {

    /// Free up Backbone atoms
    delete fixed_atoms;

    /// Free up Rotamers
    while (nrotamer)
        delete_rotamer(rotamer[0]->id);
    memory->sfree(rotamer);
}

Site::Site(const Site &s) : Pointers(s) {

    rotamer = NULL;

    id = s.id;
    resid = s.resid;
    nrotamer = s.nrotamer;
    maxrotamer = s.maxrotamer;
    strcpy(chain, s.chain);
    strcpy(seg, s.seg);
    mask = s.mask;
    ngroup_iter = s.ngroup_iter;

    // Copy instance of fixed-atoms;
    fixed_atoms = new Group(*(s.fixed_atoms));

    // Allocate memory to store copies of groups
    rotamer = (Group **) memory->srealloc(rotamer, maxrotamer * sizeof(Group *), "site:rotamer");

    // Copy instances, not pointers of groups
    for (int i = 0; i < nrotamer; i++)
        rotamer[i] = new Group(*(s.rotamer[i]));

    // Set the site pointers for the groups & atoms
    for (int i = 0; i < nrotamer; i++) {
        rotamer[i]->site = this;

        for (int j = 0; j < rotamer[i]->natom; j++)
            rotamer[i]->atom[j]->site = this;
    }

    fixed_atoms->site = this;
    for (int i = 0; i < fixed_atoms->natom; i++)
        fixed_atoms->atom[i]->site = this;
}

Site& Site::operator= (Site const& s) {

    // Don't create if self-assignment
    if (this == &s) return *this;

    // Base operator
    Pointers::operator=(s);

    //Cleanup instance being copied to
    for (int i = 0; i < nrotamer; i++)
        delete rotamer[i];
    memory->sfree(rotamer);
    rotamer = NULL;

    id = s.id;
    resid = s.resid;
    nrotamer = s.nrotamer;
    maxrotamer = s.maxrotamer;
    strcpy(chain, s.chain);
    strcpy(seg, s.seg);
    mask = s.mask;
    ngroup_iter = s.ngroup_iter;

    // Copy instance of fixed_atoms
    //fixed_atoms = new Group(*(s.fixed_atoms));
    *fixed_atoms = *(s.fixed_atoms);

    // Allocate memory to store copies of groups
    rotamer = (Group **) memory->srealloc(rotamer, maxrotamer * sizeof(Group *), "site:rotamer");

    // Copy instances, not pointers of groups
    for (int i = 0; i < nrotamer; i++)
        rotamer[i] = new Group(*(s.rotamer[i]));

    // Set the site pointers for the groups & atoms
    for (int i = 0; i < nrotamer; i++) {
        rotamer[i]->site = this;

        for (int j = 0; j < rotamer[i]->natom; j++)
            rotamer[i]->atom[j]->site = this;
    }

    fixed_atoms->site = this;
    for (int i = 0; i < fixed_atoms->natom; i++)
        fixed_atoms->atom[i]->site = this;

    return *this;
}

/**
 * Add a rotamer group to the site
 */
int Site::add_rotamer() {

    /// If we don't have enough space to add rotamer, realloc rotamer to make room.
    if (nrotamer == maxrotamer) {
        maxrotamer += ROTAMER_DELTA;
        rotamer = (Group **) memory->srealloc(rotamer, maxrotamer * sizeof(Group *), "site:rotamer");
    }

    rotamer[nrotamer] = new Group(ccb, ngroup_iter);

    nrotamer++;
    ngroup_iter++;

    return (nrotamer - 1);
}

/**
 * @brief add a rotamer to this site using the copy constructor
 *
 * @param g group instance
 *
 * @return index to group in rotamer array in site
 */
int Site::add_rotamer(Group &g) {

    /// If we don't have enough space to add rotamer, realloc rotamer to make room.
    if (nrotamer == maxrotamer) {
        maxrotamer += ROTAMER_DELTA;
        rotamer = (Group **) memory->srealloc(rotamer, maxrotamer * sizeof(Group *), "site:rotamer");
    }

    rotamer[nrotamer] = new Group(g);
    rotamer[nrotamer]->id = ngroup_iter++;

    nrotamer++;
    return (nrotamer - 1);
}


/**
 * Delete a rotamer from the site
 *
 * @param   id the romater's internal ID.
 */

int Site::delete_rotamer(int id) {

    /// Find the rotamer with the specified index
    int irotamer = find_rotamer(id);

    /// Delete the rotamer
    if (irotamer < 0)
        return error->all(FLERR, "Can't find rotamer to delete");
    delete rotamer[irotamer];

    /// Update the array of rotamer to reflect the deletion
    for (int i = irotamer + 1; i < nrotamer; i++)
        rotamer[i - 1] = rotamer[i];

    nrotamer--;

    return CCB_OK;

}

void Site::delete_all_rotamers() {

    /// Delete the rotamers
    while (nrotamer > 0)
        delete_rotamer(rotamer[0]->id);
}

/**
 * Find a rotamer by ID
 *
 * @param   id internal ID
 * @return  the identified rotamer's id.
 */

int Site::find_rotamer(int id) {
    int irotamer;

    for (irotamer = 0; irotamer < nrotamer; irotamer++)
        if (id == rotamer[irotamer]->id)
            break;
    if (irotamer == nrotamer)
        return -1;
    return irotamer;
}

double Site::memory_usage() {

    double bytes = (nrotamer + 1) * sizeof(Group);
    bytes += (maxrotamer + 1) * sizeof(Group *);

    return bytes;
}
