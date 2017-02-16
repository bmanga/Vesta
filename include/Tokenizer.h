#pragma once
#include <string>
#include <cassert>
#include <vector>
#include <algorithm>

class Document;
class Cursor;
class TextWindow;

typedef void(*CallbackT) (TextWindow *, char);

class Trigger {
public:
	enum ActivationKind : uint8_t {
		Immediate = 4,
		Tab = 1,
		Enter = 2,
		Either = 3,
		Global = 0
	};

	Trigger(std::string Value, CallbackT CB, ActivationKind AK = Global, bool KeySeqMatch = false, bool RequiresStreaming = false)
		: mValue(std::move(Value))
		, mCallback(CB)
		, mActivationKind(AK)
		, mStreamNext(RequiresStreaming)
		, mExactKeySeqMatch(KeySeqMatch) {}

	size_t length() const {
		return mValue.size();
	}

	char charAt(size_t Idx) {
		assert(Idx < length());

		return mValue[Idx];
	}

	void fire(TextWindow *TW, char C) {
		mCallback(TW, C);
	}

	ActivationKind getActivationKind() {
		return mActivationKind;
	}

	bool requiresStreaming() const {
		return mStreamNext;
	}

	bool exactKeySeqMatchOnly() const {
		return mExactKeySeqMatch;
	}
private:

private:
	// If a trigger is activated, stream the following chars to the callback
	bool mStreamNext = false;
	bool mExactKeySeqMatch; // Whether undos are allowed

	ActivationKind mActivationKind = ActivationKind::Immediate;
	std::string mValue;
	CallbackT mCallback;
};

/*
* Deals with the actual trigger behavior. Holds the state of the trigger
* it is bound with. 
*/

class TriggerOpt
{
public:
	TriggerOpt(Trigger *T)
		: mTrigger(T) {}

	bool eval(char C) {
		if (mInvalid) {
			return false;
		}

		// Deal with backspaces
		if (!mTrigger->exactKeySeqMatchOnly() && C == 8) {
			if (mIdx) --mIdx;
			return true;
		}

		if (mIdx >= mTrigger->length()) {
			mReady = false;
			mInvalid = true;
			mIdx = 0;
		}

		bool Ok = mTrigger->charAt(mIdx) == C;

		++mIdx;

		// The trigger is activated
		if (Ok && mIdx == mTrigger->length()) {
			mReady = true;
		}

		if (!Ok) {
			mReady = false;
			mInvalid = true;
			mIdx = 0;
		}
		return Ok;
	}
	// Try to fire the trigger.
	// \Return true if successful.
	bool activate(Trigger::ActivationKind AK, TextWindow *TK, char C) {
		if (!mReady) {
			return false;
		}
		if (!(mTrigger->getActivationKind() & AK)) {
			return false;
		}

		mTrigger->fire(TK, C);
		if (!mTrigger->requiresStreaming()) {
			reset();
		}
		
		return true;
	}

	void reset() {
		mInvalid = false;
		mReady = false;
		mIdx = 0;
	}
private:
	bool mInvalid = false;
	bool mReady = false;
	Trigger *mTrigger;
	unsigned mIdx = 0;
};

// Check what the user types and identify possible trigger tokens.
// Uses callbacks to process given 
class TriggerLex
{
	
public:
	TriggerLex() = default;
	TriggerLex(std::initializer_list<Trigger> Triggers) 
		: mTriggers(Triggers)
	{
		mCandidates.reserve(Triggers.size());

		for (size_t j = 0; j < mTriggers.size(); ++j) {
			mCandidates.emplace_back(&mTriggers[j]);
		}
	}

	void add(Trigger T) {
		mTriggers.push_back(std::move(T));

		mCandidates.clear();
		for (size_t j = 0; j < mTriggers.size(); ++j) {
			mCandidates.emplace_back(&mTriggers[j]);
		}
	}
	// Process the provided character for non-immediate triggers.
	// \Return true if any of the triggers fired
	// and the character should be swallowed.
	bool processAndSwallow(TextWindow *TW, char Char) {
		bool FiredOne = false;

		// Space resets every Trigger
		// FIXME this is probably not what we want
		if (Char == ' ') {
			for (auto && Candidate : mCandidates) {
				Candidate.reset();
			}
			return false;
		}

		if (Char == '\t' || Char == '\n') {
			Trigger::ActivationKind AK;
			switch (Char) {
			case '\t': AK = Trigger::ActivationKind::Tab;   break;
			case '\n': AK = Trigger::ActivationKind::Enter; break;
			}

			for (auto &&Candidate : mCandidates) {
				FiredOne |= Candidate.activate(AK, TW, Char);
				Candidate.reset();
			}

			return FiredOne;
		}
		
		// TODO: Optimization opportunity : 
		//	we only need to iterate over the "good" candidates.
		//	Consider using std::partition and only going through the good parts
		for (auto &&Candidate : mCandidates) {
			Candidate.eval(Char);
		}

		return false;
	}

	// Process the provided character for immediate triggers.
	// \Return true if any of the triggers fired
	bool processImmediates(TextWindow *TW, char Char) {
		bool FiredOne = false;
		// Run the immediate triggers
		for (auto &&Candidate : mCandidates) {
			FiredOne |= Candidate.activate(Trigger::ActivationKind::Immediate, TW, Char);
		}

		return FiredOne;
	}
private:
	std::vector<Trigger> mTriggers;
	std::vector<TriggerOpt> mCandidates;

};