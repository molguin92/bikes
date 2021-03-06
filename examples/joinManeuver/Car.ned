//
// Copyright (C) 2008 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

import org.car2x.veins.base.modules.IBaseApplLayer;
import org.car2x.veins.modules.application.platooning.protocols.BaseProtocol;
import org.car2x.veins.modules.application.platooning.UnicastProtocol;
import org.car2x.veins.modules.mobility.traci.TraCIMobility;
import org.car2x.veins.modules.nic.Nic80211p;


module Car
{
    parameters:
        string appl_type;
        string protocol_type;

    gates:
        input radioIn; // gate for sendDirect
    submodules:

        appl: <appl_type> like IBaseApplLayer {
            parameters:
                @display("p=60,100");
        }

        prot: <protocol_type> like BaseProtocol {
            parameters:
                @display("p=60,200");
        }

        unicast: UnicastProtocol {
            parameters:
                @display("p=60,300");
        }

        nic: Nic80211p {
            parameters:
                @display("p=60,400");
        }

        mobility: TraCIMobility {
            parameters:
                @display("p=130,172;i=block/cogwheel");
        }
    connections allowunconnected:
        prot.upperLayerIn <-- appl.lowerLayerOut;
        prot.upperLayerOut --> appl.lowerLayerIn;
        appl.lowerControlOut --> prot.upperControlIn;
        appl.lowerControlIn <-- prot.upperControlOut;
        unicast.upperControlIn <-- prot.lowerControlOut;
        unicast.upperControlOut --> prot.lowerControlIn;
        unicast.upperLayerIn <-- prot.lowerLayerOut;
        unicast.upperLayerOut --> prot.lowerLayerIn;
        nic.upperLayerIn <-- unicast.lowerLayerOut;
        nic.upperLayerOut --> unicast.lowerLayerIn;

        radioIn --> nic.radioIn;

}
