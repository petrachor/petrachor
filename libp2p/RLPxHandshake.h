/*
 This file is part of cpp-ethereum.
 
 cpp-ethereum is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 cpp-ethereum is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file RLPXHandshake.h
 * @author Alex Leverington <nessence@gmail.com>
 * @date 2015
 */


#pragma once

#include <memory>
#include <libdevcrypto/Common.h>
#include "RLPXSocket.h"
#include "RLPXFrameCoder.h"
#include "Common.h"
namespace ba = boost::asio;
namespace bi = boost::asio::ip;

namespace dev
{
namespace p2p
{

typedef dev::crypto::Keys RLPXKeys;

static const unsigned c_rlpxVersion = 4;

/**
 * @brief Setup inbound or outbound connection for communication over RLPXFrameCoder.
 * RLPx Spec: https://github.com/ethereum/devp2p/blob/master/rlpx.md#encrypted-handshake
 *
 * @todo Implement StartSession transition via lambda which is passed to constructor.
 *
 * Thread Safety
 * Distinct Objects: Safe.
 * Shared objects: Unsafe.
 */
class RLPXHandshake: public std::enable_shared_from_this<RLPXHandshake>
{
	friend class RLPXFrameCoder;

public:
	/// Setup incoming connection.
	RLPXHandshake(Host* _host, std::shared_ptr<RLPXSocket> const& _socket): m_host(_host), m_originated(false), m_socket(_socket), m_idleTimer(m_socket->ref().get_executor()) { crypto::Nonce::get().ref().copyTo(m_nonce.ref()); }
	
	/// Setup outbound connection.
	RLPXHandshake(Host* _host, std::shared_ptr<RLPXSocket> const& _socket, NodeID _remote): m_host(_host), m_remote(_remote), m_originated(true), m_socket(_socket), m_idleTimer(m_socket->ref().get_executor()) { crypto::Nonce::get().ref().copyTo(m_nonce.ref()); }

	virtual ~RLPXHandshake() = default;

	/// Start handshake.
	void start() { transition(); }

	/// Aborts the handshake.
	void cancel();

protected:
	/// Sequential states of handshake
	enum State
	{
		Error = -1,
		New,
		AckAuth,
		AckAuthEIP8,
		WriteHello,
		ReadHello,
		StartSession
	};

	/// Write Auth message to socket and transitions to AckAuth.
	void writeAuth();

	/// Reads Auth message from socket and transitions to AckAuth.
	void readAuth();

	/// Continues reading Auth message in EIP-8 format and transitions to AckAuthEIP8.
	void readAuthEIP8();

	/// Derives ephemeral secret from signature and sets members after Auth has been decrypted.
    void setAuthValues(ECDSA::Signature const& sig, ECDSA::Public const& remotePubk, h256 const& remoteNonce, uint64_t remoteVersion);
	
	/// Write Ack message to socket and transitions to WriteHello.
	void writeAck();

	/// Write Ack message in EIP-8 format to socket and transitions to WriteHello.
	void writeAckEIP8();
	
	/// Reads Auth message from socket and transitions to WriteHello.
	void readAck();

	/// Continues reading Ack message in EIP-8 format and transitions to WriteHello.
	void readAckEIP8();
	
	/// Closes connection and ends transitions.
	void error();
	
	/// Performs transition for m_nextState.
	virtual void transition(boost::system::error_code _ech = boost::system::error_code());

	/// Timeout for remote to respond to transition events. Enforced by m_idleTimer and refreshed by transition().
	boost::posix_time::milliseconds const c_timeout = boost::posix_time::milliseconds(1800);

	State m_nextState = New;		///< Current or expected state of transition.
	bool m_cancel = false;			///< Will be set to true if connection was canceled.
	
	Host* m_host;					///< Host which provides m_alias, protocolVersion(), m_clientVersion, caps(), and TCP listenPort().
	
	/// Node id of remote host for socket.
	NodeID m_remote;				///< Public address of remote host.
	bool m_originated = false;		///< True if connection is outbound.
	
	/// Buffers for encoded and decoded handshake phases
	bytes m_auth;					///< Plaintext of egress or ingress Auth message.
	bytes m_authCipher;				///< Ciphertext of egress or ingress Auth message.
	bytes m_ack;					///< Plaintext of egress or ingress Ack message.
	bytes m_ackCipher;				///< Ciphertext of egress or ingress Ack message.
	bytes m_handshakeOutBuffer;		///< Frame buffer for egress Hello packet.
	bytes m_handshakeInBuffer;		///< Frame buffer for ingress Hello packet.
	
    RLPXKeys::Pair m_ecdheLocal = RLPXKeys::Pair::create();  ///< Ephemeral ECDH secret and agreement.
	h256 m_nonce;					///< Nonce generated by this host for handshake.
	
    RLPXKeys::Public m_ecdheRemote;			///< Remote ephemeral public key.
	h256 m_remoteNonce;				///< Nonce generated by remote host for handshake.
	uint64_t m_remoteVersion;
	
	/// Used to read and write RLPx encrypted frames for last step of handshake authentication.
	/// Passed onto Host which will take ownership.
	std::unique_ptr<RLPXFrameCoder> m_io;
	
	std::shared_ptr<RLPXSocket> m_socket;		///< Socket.
	boost::asio::deadline_timer m_idleTimer;	///< Timer which enforces c_timeout.
};
	
}
}
