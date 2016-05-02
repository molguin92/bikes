#ifndef BikeManeuver_H_
#define BikeManeuver_H_

#include "veins/modules/application/platooning/apps/BaseApp.h"
#include "veins/modules/application/platooning/messages/BikesMessage_m.h"

#define TCAR 0
#define TBIKE 1


class BikeManeuver : public BaseApp
{
	protected:
		enum ROLE { LEADER, FOLLOWER, BIKE };
		struct VEHICLE_DATA
		{
			int platoonId;              //id of the platoon
			int leaderId;               //leader of the platoon
			int frontId;                //id of the vehicle in front
			double speed;               //speed of the platoon
			int lane;                   //index of the lane the platoon is travelling in
			int leaverId;               //the id of the vehicle leaving the platoon
			int isBlinking;             //see if i'm turning -> will become a leaver!
			std::vector < int > formation;  //list of vehicles in the platoon
		};

		typedef enum _LEADER_STATES
		{
			LS_INIT = 0,
			LS_IDLE = FSM_Steady( 1 ),
			LS_BROADCAST = FSM_Steady( 2 )
		} LEADER_STATES;

		typedef enum _FOLLOWER_STATES
		{
			FS_INIT = 0,
			FS_IDLE = FSM_Steady( 1 ),
		} FOLLOWER_STATES;

		typedef enum _BIKE_STATES
		{
			BS_INIT = 0,
			BS_IDLE = FSM_Steady( 1 ),
			BS_WARNING = FSM_Steady( 2 )
		} BIKE_STATES;

        enum LEADER_MSGS {
            LM_WARNING = 0,
            LM_TURNING = 1,
            LM_DONE_TURNING = 2
        };

        cFSM leaderFSM, followerFSM, bikeFSM;
        ROLE myRole;
        int position;
        struct VEHICLE_DATA vehicleData;
        cMessage *startSim;
        int myType;

    public:
        virtual void initialize(int stage);
        virtual void finish();
		BikeManeuver()
		{
			startSim = 0;
		}

	protected:
		virtual void onBeacon(WaveShortMessage* wsm);
		virtual void onData(WaveShortMessage* wsm);

		virtual void handleSelfMsg(cMessage *msg);
		//override this method of BaseApp. we want to handle it ourself
		virtual void handleLowerMsg(cMessage *msg);
		virtual void handleLowerControl(cMessage *msg);

		BikesMessage *generateMessage();

		void handleLeaderMsg(cMessage *msg);
		void handleJoinerMsg(cMessage *msg);
		void handleFollowerMsg(cMessage *msg);

		void prepareManeuverCars(int platoonLane);
		void receiveMessage(cMessage *msg);
};

#endif
