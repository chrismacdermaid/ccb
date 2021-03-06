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
 * @file   group.h
 * @author Chris Von Bargen <cvb@magrathea.chem.upenn.edu>
 * @date   Nov 14 2011
 *
 * @brief  Group, describing a collection of atoms
 *
 * Class to describe container for a collection of atoms, will fields
 * filled in as necessary. To be used(extended) as a static fixed_atoms group,
 * or a series of rotameric states.
 *
 */

#ifndef CCB_GROUP_H
#define CCB_GROUP_H

#include "pointers.h"

namespace CCB_NS {
class Group: protected Pointers {

  public:

    // Constructor and Destructor
    Group(class CCB *, int i); /**< Site constructor */
    ~Group(); /**< Site deconstructor */
    Group(const Group &); /**< Copy Constructor */
    Group& operator=(Group const &); /**< assignment operator */

    int id; /**< Internal indexing counter */
    char name[15]; /**< group name */
    char type[15]; /**< group type, ALA, ASP etc.. */

    class Atom **atom; /**< list of atoms */
    int natom; /**< Total number of atoms in static part */

    class Site *site; /**< Pointer to the site this group belongs to */

    // Functions to manage atoms
    int add_atom();
    int add_atom(Atom *a);
    int overwrite_atom(Atom *a);
    int add_atom(Atom &a); /**< add an atom using the copy constructor */
    int delete_atom(int id);
    int delete_atom_name(const char *name);
    int find_atom(int id);
    int find_atom_name(const char *name);

    double memory_usage();

  private:
    bigint natom_iter; /** < The unique atom id iterator, this should only be increased */
    int maxatom; /**< Maximum number of atoms in based on currently allocated space */
};
}

#endif

