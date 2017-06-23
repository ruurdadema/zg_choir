#ifndef ZGPeerSettings_h
#define ZGPeerSettings_h

#include "zg/ZGNameSpace.h"
#include "message/Message.h"

/** The zg_private namespace is an undocumented namespace where the ZG library keeps all of its private implementation details that user programs aren't supposed to access directly. */
namespace zg_private
{
   class PZGHeartbeatThreadState;  // forward declaration
};

namespace zg
{

enum {
   PEER_TYPE_FULL_PEER = 0,  // a full peer is one that is able to become senior peer if necessary
   PEER_TYPE_JUNIOR_ONLY,    // a junior-only peer is one that wants to follow along as a junior peer but doesn't want to be senior
   NUM_PEER_TYPES
};

/** This immutable class holds various read-only settings that will be used to define the
  * peer's behavior.  These settings are not allowed to change during the lifetime of the peer.
  */
class ZGPeerSettings
{
public:
   /** Constructor. 
     * @param systemName Name of the system this peer will participate in.  All peers in a system by definition use the same system name.
     * @param numDatabases The number of replicated databases this system should maintain.
     * @param systemIsOnLocalhostOnly If true, we'll send/receive multicast packets on loopback interfaces only.  If false, we'll use all interfaces.
     * @param peerType One of the PEER_TYPE_* values.  Defaults to PEER_TYPE_FULL_PEER, meaning that this peer is willing to handle
     *                 both junior-peer and senior-peer duties, if necessary.
     */
   ZGPeerSettings(const String & systemName, uint8 numDatabases, bool systemIsOnLocalhostOnly, uint16 peerType = PEER_TYPE_FULL_PEER)
      : _systemName(systemName)
      , _numDatabases(numDatabases)
      , _systemIsOnLocalhostOnly(systemIsOnLocalhostOnly)
      , _peerType(peerType)
      , _heartbeatsPerSecond(6)  // setting this at >5 avoids the great MacOS/X WiFi-PowerSave-on-200mS-idle problem
      , _heartbeatsBeforeFullyAttached(4)
      , _maxMissingHeartbeats(4)
      , _beaconsPerSecond(4)
      , _outgoingHeartbeatPacketIDCounter(0)
   {
      // empty
   }

   /** Returns the ZG system's system name (as specified in our constructor) */
   const String & GetSystemName()              const {return _systemName;}

   /** Returns the number of databases that will be present in this ZG system (as specified by our constructor) */
   uint8 GetNumDatabases()                     const {return _numDatabases;}

   /** Returns true iff this ZG system is meant to be contained entirely within a single host computer (as specified in our constructor) */
   bool IsSystemOnLocalhostOnly()              const {return _systemIsOnLocalhostOnly;}

   /** Returns a reference to this peer's peer-attributes Message (as specified earlier via SetPeerAttributes()).  
     * May return a NULL reference if no peer-attributes were ever specified.
     */
   const ConstMessageRef & GetPeerAttributes() const {return _optPeerAttributes;}

   /** Returns the PEER_TYPE_* value of this peer (currently only PEER_TYPE_FULL_PEER is supported) */
   uint16 GetPeerType()                        const {return _peerType;}

   /** Returns the heartbeats-per-second value for htis peer (currently defaults to 6) */
   uint32 GetHeartbeatsPerSecond()             const {return _heartbeatsPerSecond;}

   /** Returns the max-missing-heartbeats-before-a-peer-is-declared-offline value for this peer (currently defaults to 4) */
   uint32 GetMaxNumMissingHeartbeats()         const {return _maxMissingHeartbeats;}

   /** Returns the num-heartbeats-before-peer-considers-itself-fully-attached value this peer (currently defaults to 4) */
   uint32 GetHeartbeatsBeforeFullyAttached()   const {return _heartbeatsBeforeFullyAttached;}

   /** Returns the number-of-beacon-packets-to-send-per-second value for this peer (currently defaults to 4) */
   uint32 GetBeaconsPerSecond()                const {return _beaconsPerSecond;}

   /** Set the peer attributes that should be associated with this peer.  Default is to have no attributes. 
     * @param peerAttributes Reference to a Message containing our attributes.  Try to keep this small, as a
     *                       zlib-compressed copy of this Message will be included in every heartbeat packet we send!
     */
   void SetPeerAttributes(const ConstMessageRef & peerAttributes) {_optPeerAttributes = peerAttributes;}

   /** Set the number of times we should send a "heartbeat" multicast packet per second.
     * This value will also be used as the number of heartbeats we expect other peers in our group to send,
     * so it is important that this value be the same on all peers in the sysetm
     * Default value is 4 (i.e. one heartbeat every 250 milliseconds)
     * @param hps The number of heartbeats per second to send, and to expect to receive.  If set to 0, we'll act as if it was set to 1.
     */
   void SetHeartbeatsPerSecond(uint32 hps) {_heartbeatsPerSecond = hps;}

   /** Sets the number of heartbeat-periods that must elapse after our startup, before we declare ourselves
     * fully part of the peers group.  During this initial period we simply listen to find out who else is
     * currently out there and passively gather data about them, without trying to interfere with the operation
     * of the existing system.
     * Default value is 4 (i.e. we will wait for four heartbeat-intervals before trying to act as a full member of the system)
     * @param hb The number of heartbeat intervals to wait for during our startup phase.
     */
   void SetHeartbeatsBeforeFullyAttached(uint32 hb) {_heartbeatsBeforeFullyAttached = hb;}

   /** Sets the number of heartbeat-periods that must go by without hearing any heartbeats from a source,
     * before we will decide that that source has gone offline.
     * Default value is 4 (i.e. we will wait for four heartbeat-intervals to go by without any heartbeats from a source, before declaring a source to be offline)
     * @param hb The number of heartbeat intervals that will trigger an offline-state.
     */
   void SetMaxNumMissingHeartbeats(uint32 hb) {_maxMissingHeartbeats = hb;}

   /** Set the number of times we should send a "beacon" multicast packet per second, if we are the senior peer.
     * Beacon packets are used by the junior peers to verify that they haven't missed an update.  They are sent only by the senior peer.
     * Default value is 4 (i.e. one beacon packet every 250 milliseconds)
     * @param bps The number of beacons per second to send (only if we are the senior peer).
     */
   void SetBeaconsPerSecond(uint32 bps) {_beaconsPerSecond = bps;}

   /** Call this to set the maximum number of bytes of RAM the specified database should be allowed
     * to use for its database-update-log records.  If not specified for a given database, a default
     * limit of two megabytes will be used.
     * @param whichDB The database you want to specify a new limit for
     * @param maxNumBytes The new limit, in bytes.  If set to 0, then the default limit will be reinstated for that database.
     */
   void SetMaximumUpdateLogSizeForDatabase(uint32 whichDB, uint64 maxNumBytes) {(void) _maxUpdateLogSizeBytes.PutOrRemove(whichDB, maxNumBytes);}

   /** Returns the maximum number of bytes of RAM we should be allowed to use for the update-log of the specified database.
     * @param whichDB The database you want to retreive the RAM-usage limit for
     */
   uint64 GetMaximumUpdateLogSizeForDatabase(uint32 whichDB) const {return _maxUpdateLogSizeBytes.GetWithDefault(whichDB, 2*1024*1024);}

private:
#ifndef DOXYGEN_SHOULD_IGNORE_THIS
   friend class zg_private::PZGHeartbeatThreadState;
#endif

   String _systemName;                 // Name of the ZG system we are to participate in
   uint8 _numDatabases;                // how many databases we want to maintain
   bool _systemIsOnLocalhostOnly;      // true iff we are simulating the system on a single host
   ConstMessageRef _optPeerAttributes; // optional user-specified descriptive attributes for this peer (should be small)
   uint16 _peerType;                   // PEER_TYPE_* value for this peer
   uint32 _heartbeatsPerSecond;        // how many heartbeats we should send per second (and expect to receive per second, from each peer)
   uint32 _heartbeatsBeforeFullyAttached;  // how many heartbeat-periods we should allow to elapse before declaring ourselves fully part of the system.
   uint32 _maxMissingHeartbeats;       // how many heartbeat-periods must go by without receiving a heartbeat from a peer, before we declare him offline
   uint32 _beaconsPerSecond;           // how many beacon-packets we should send out per second if we are the senior peer
   Hashtable<uint32, uint64> _maxUpdateLogSizeBytes;
   mutable uint32 _outgoingHeartbeatPacketIDCounter;
};

};  // end namespace zg

#endif
