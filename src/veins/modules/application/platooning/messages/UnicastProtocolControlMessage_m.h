//
// Generated file, do not edit! Created by nedtool 4.6 from veins/modules/application/platooning/messages/UnicastProtocolControlMessage.msg.
//

#ifndef _UNICASTPROTOCOLCONTROLMESSAGE_M_H_
#define _UNICASTPROTOCOLCONTROLMESSAGE_M_H_

#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0406
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



/**
 * Class generated from <tt>veins/modules/application/platooning/messages/UnicastProtocolControlMessage.msg:22</tt> by nedtool.
 * <pre>
 * //
 * // Control message sent from the application layer to the unicast protocol
 * // or vicevarsa
 * //
 * packet UnicastProtocolControlMessage
 * {
 *     //control command requested (e.g., set mac address or fail to send)
 *     int controlCommand;
 *     //value for the control command (e.g., actual mac address)
 *     int commandValue;
 * }
 * </pre>
 */
class UnicastProtocolControlMessage : public ::cPacket
{
  protected:
    int controlCommand_var;
    int commandValue_var;

  private:
    void copy(const UnicastProtocolControlMessage& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const UnicastProtocolControlMessage&);

  public:
    UnicastProtocolControlMessage(const char *name=NULL, int kind=0);
    UnicastProtocolControlMessage(const UnicastProtocolControlMessage& other);
    virtual ~UnicastProtocolControlMessage();
    UnicastProtocolControlMessage& operator=(const UnicastProtocolControlMessage& other);
    virtual UnicastProtocolControlMessage *dup() const {return new UnicastProtocolControlMessage(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual int getControlCommand() const;
    virtual void setControlCommand(int controlCommand);
    virtual int getCommandValue() const;
    virtual void setCommandValue(int commandValue);
};

inline void doPacking(cCommBuffer *b, UnicastProtocolControlMessage& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, UnicastProtocolControlMessage& obj) {obj.parsimUnpack(b);}


#endif // ifndef _UNICASTPROTOCOLCONTROLMESSAGE_M_H_

