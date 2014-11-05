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
/** @file Transaction.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libdevcore/RLP.h>
#include <libdevcrypto/SHA3.h>
#include <libethcore/CommonEth.h>

namespace dev
{
namespace eth
{

class Transaction
{
public:
	enum Type
	{
		NullTransaction,
		ContractCreation,
		MessageCall
	};

	Transaction() {}
	Transaction(u256 _value, u256 _gasPrice, u256 _gas, Address const& _dest, bytes const& _data, u256 _nonce, Secret const& _secret): m_type(MessageCall), m_nonce(_nonce), m_value(_value), m_receiveAddress(_dest), m_gasPrice(_gasPrice), m_gas(_gas), m_data(_data) { sign(_secret); }
	Transaction(u256 _value, u256 _gasPrice, u256 _gas, bytes const& _data, u256 _nonce, Secret const& _secret): m_type(ContractCreation), m_nonce(_nonce), m_value(_value), m_gasPrice(_gasPrice), m_gas(_gas), m_data(_data) { sign(_secret); }
	Transaction(u256 _value, u256 _gasPrice, u256 _gas, Address const& _dest, bytes const& _data): m_type(MessageCall), m_value(_value), m_receiveAddress(_dest), m_gasPrice(_gasPrice), m_gas(_gas), m_data(_data) {}
	Transaction(u256 _value, u256 _gasPrice, u256 _gas, bytes const& _data): m_type(ContractCreation), m_value(_value), m_gasPrice(_gasPrice), m_gas(_gas), m_data(_data) {}
	Transaction(bytesConstRef _rlp, bool _checkSender = false);
	Transaction(bytes const& _rlp, bool _checkSender = false): Transaction(&_rlp, _checkSender) {}

	bool operator==(Transaction const& _c) const { return m_type == _c.m_type && (m_type == ContractCreation || m_receiveAddress == _c.m_receiveAddress) && m_value == _c.m_value && m_data == _c.m_data; }
	bool operator!=(Transaction const& _c) const { return !operator==(_c); }

	Address safeSender() const noexcept;	///< Like sender() but will never throw.
	Address sender() const;					///< Determine the sender of the transaction from the signature (and hash).
	void sign(Secret _priv);				///< Sign the transaction.

	bool isCreation() const { return !m_receiveAddress; }

	void streamRLP(RLPStream& _s, bool _sig = true) const;

	bytes rlp(bool _sig = true) const { RLPStream s; streamRLP(s, _sig); return s.out(); }
	std::string rlpString(bool _sig = true) const { return asString(rlp(_sig)); }
	h256 sha3(bool _sig = true) const { RLPStream s; streamRLP(s, _sig); return dev::sha3(s.out()); }
	bytes sha3Bytes(bool _sig = true) const { RLPStream s; streamRLP(s, _sig); return dev::sha3Bytes(s.out()); }

	Type type() const { return m_type; }
	u256 nonce() const { return m_nonce; }
	u256 value() const { return m_value; }
	Address receiveAddress() const { return m_receiveAddress; }
	u256 gasPrice() const { return m_gasPrice; }
	u256 gas() const { return m_gas; }
	bytes const& data() const { return m_data; }
	SignatureStruct const& signature() const { return m_vrs; }

private:
	Type m_type = NullTransaction;	///< True if this is a contract-creation transaction. F
	u256 m_nonce;					///< The transaction-count of the sender.
	u256 m_value;					///< The amount of ETH to be transferred by this transaction. Called 'endowment' for contract-creation transactions.
	Address m_receiveAddress;		///< The receiving address of the transaction.
	u256 m_gasPrice;				///< The base fee and thus the implied exchange rate of ETH to GAS.
	u256 m_gas;						///< The total gas to convert, paid for from sender's account. Any unused gas gets refunded once the contract is ended.
	bytes m_data;					///< The data associated with the transaction, or the initialiser if it's a creation transaction.
	SignatureStruct m_vrs;			///< The signature of the transaction. Encodes the sender.

	mutable Address m_sender;
};

using Transactions = std::vector<Transaction>;

inline std::ostream& operator<<(std::ostream& _out, Transaction const& _t)
{
	_out << "{";
	if (_t.receiveAddress())
		_out << _t.receiveAddress().abridged();
	else
		_out << "[CREATE]";

	_out << "/" << _t.nonce() << "$" << _t.value() << "+" << _t.gas() << "@" << _t.gasPrice();
	try
	{
		_out << "<-" << _t.sender().abridged();
	}
	catch (...) {}
	_out << " #" << _t.data().size() << "}";
	return _out;
}

}
}
