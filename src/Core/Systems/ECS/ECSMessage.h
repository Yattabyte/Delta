/*
	ECSmessage

	- A class for sending data to and from components and entities.
	- Is a container for any data type
	- Data type can be manually checked, and specifically typecasted on request
*/

#pragma once
#ifndef ECSMESSAGE
#define ECSMESSAGE
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include <stdio.h>
#include <string>
#include <utility>
#include <memory>

typedef std::pair<char*, unsigned int> ECSHandle;

using namespace std;

class DELTA_CORE_API ECSmessage
{
private:
	// The interface / base class
	struct PayloadConcept {
		virtual ~PayloadConcept() {};
		virtual const char* GetTypeID() = 0;
	};

	// The specific class that will hold our object
	template <typename T>
	struct PayloadModel : PayloadConcept {
		PayloadModel(const T& t) { m_data = t; }
		~PayloadModel() {}
		const char* GetTypeID() {
			return typeid(T).name();
		}
		const T &GetData() const { return m_data; }
	private:
		T m_data;
	};

	ECSHandle m_senderID, m_targetID;
	std::shared_ptr<PayloadConcept> m_payload;


public:
	// Constructor, generates a container to hold our object
	template< typename T > 
	ECSmessage(const T& obj) : m_payload(new PayloadModel<T>(obj)) {}
	~ECSmessage() { }

	ECSHandle GetSenderID() const { return m_senderID; };
	void SetSenderID(const ECSHandle &sender) { m_senderID = sender; };
	ECSHandle GetTargetID() const { return m_targetID; };
	void SetTargetID(const ECSHandle &target) { m_targetID = target; };

	// Checks to see if the payload is of the type supplied
	template< typename T> const T &GetPayload() const {	return (std::dynamic_pointer_cast<PayloadModel<T>>(m_payload)).get()->GetData(); };	

	// Returns the payload in the format requested
	template< typename T> bool IsOfType() const { return strcmp(m_payload->GetTypeID(), typeid(T).name()) == 0; }
};

#endif // ECSMESSAGE
