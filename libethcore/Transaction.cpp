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
/** @file TransactionBase.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include <libdevcore/vector_ref.h>
#include <libdevcore/Log.h>
#include <libdevcrypto/Common.h>
#include <libethcore/Exceptions.h>
#include "Transaction.h"
#include "EVMSchedule.h"

using namespace std;
using namespace dev;
using namespace dev::eth;

TransactionBase::TransactionBase(TransactionSkeleton const& _ts, AccountKeys::Secret const& _s):
	m_type(_ts.creation ? ContractCreation : MessageCall),
	m_nonce(_ts.nonce),
	m_value(_ts.value),
	m_receiveAddress(_ts.to),
	m_gasPrice(_ts.gasPrice),
	m_gas(_ts.gas),
    m_data(_ts.data)
{
	if (_s)
        sign(_s);
}

TransactionBase::TransactionBase(bytesConstRef _rlpData, CheckTransaction _checkSig)
{
	RLP rlp(_rlpData);
	try
	{
        //clog << rlp.itemCount() << "\n";
        RLPList s(rlp, 8);
        s >> m_nonce >> m_gasPrice >> m_gas;
        RLP const& recvAField = s.pop();
		m_type = recvAField.isEmpty() ? ContractCreation : MessageCall;
		m_receiveAddress = recvAField.isEmpty() ? Address() : recvAField.toHash<Address>(RLP::VeryStrict);
        bytes chainIdBytes;
        s >> m_value >> m_data >> chainIdBytes;
        m_chainId = (char) chainIdBytes[0];
        s >> m_vrs;
        if (_checkSig >= CheckTransaction::Cheap && !m_vrs->isValid())
            BOOST_THROW_EXCEPTION(InvalidSignature());
	}
	catch (Exception& _e)
	{
		_e << errinfo_name("invalid transaction format: " + toString(rlp) + " RLP: " + toHex(rlp.data()));
		throw;
	}
}

AccountKeys::SignatureStruct const& TransactionBase::signature() const
{ 
	if (!m_vrs)
		BOOST_THROW_EXCEPTION(TransactionIsUnsigned());

	return *m_vrs;
}

void TransactionBase::sign(AccountKeys::Secret const& _priv)
{
    auto sig = dev::sign<AccountKeys::Type>(_priv, sha3(WithoutSignature));
    AccountKeys::SignatureStruct sigStruct = AccountKeys::SignatureStruct(sig, toPublic<AccountKeys::Type>(_priv));
	if (sigStruct.isValid())
		m_vrs = sigStruct;
}

void TransactionBase::streamRLP(RLPStream& _s, IncludeSignature _sig, bool _forEip155hash) const
{
	if (m_type == NullTransaction)
		return;

	_s.appendList((_sig || _forEip155hash ? 1 : 0) + 7);
	_s << m_nonce << m_gasPrice << m_gas;
	if (m_type == MessageCall)
		_s << m_receiveAddress;
	else
		_s << "";
    _s << m_value << m_data;
    bytes chainIdBytes(1);
    chainIdBytes[0] = m_chainId;
    _s << chainIdBytes;

    if (_sig)
	{
		if (!m_vrs)
			BOOST_THROW_EXCEPTION(TransactionIsUnsigned());

        m_vrs->streamRLP(_s);
	}
}

void TransactionBase::checkLowS() const
{
	if (!m_vrs)
		BOOST_THROW_EXCEPTION(TransactionIsUnsigned());

    if (m_vrs->lowS())
		BOOST_THROW_EXCEPTION(InvalidSignature());
}

void TransactionBase::checkChainId(int chainId) const
{
	if (m_chainId != chainId && m_chainId != -4)
		BOOST_THROW_EXCEPTION(InvalidSignature());
}

int64_t TransactionBase::baseGasRequired(bool _contractCreation, bytesConstRef _data, EVMSchedule const& _es)
{
	int64_t g = _contractCreation ? _es.txCreateGas : _es.txGas;

	// Calculate the cost of input data.
	// No risk of overflow by using int64 until txDataNonZeroGas is quite small
	// (the value not in billions).
	for (auto i: _data)
		g += i ? _es.txDataNonZeroGas : _es.txDataZeroGas;
	return g;
}

h256 TransactionBase::sha3(IncludeSignature _sig) const
{
	if (_sig == WithSignature && m_hashWith)
		return m_hashWith;

	RLPStream s;
	streamRLP(s, _sig, m_chainId > 0 && _sig == WithoutSignature);

	auto ret = dev::sha3(s.out());
	if (_sig == WithSignature)
		m_hashWith = ret;
	return ret;
}
