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

	Trigger(std::string Value, CallbackT CB, ActivationKind AK = Global, bool RequiresStreaming = false) 
		: mValue(std::move(Value))
		, mCallback(CB)
		, mActivationKind(AK)
		, mStreamNext(RequiresStreaming){}

	size_t length() const {
		return mValue.size();
	}

	char charAt(size_t Idx) {
		assert(Idx < length());

		return mValue[Idx];
	}

	void activate(TextWindow *TW, char C) {
		mCallback(TW, C);
	}

	ActivationKind getActivationKind() {
		return mActivationKind;
	}

	bool requiresStreaming() const {
		return mStreamNext;
	}
private:

private:
	// if a trigger is activated, stream the following chars to the callback
	bool mStreamNext = false;
	ActivationKind mActivationKind = ActivationKind::Immediate;
	std::string mValue;
	CallbackT mCallback;
};



class TriggerOpt
{
public:
	TriggerOpt(Trigger *T)
		: mTrigger(T) {}

	bool eval(char C) {
		if (mInvalid) {
			return false;
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
	
	void activate(Trigger::ActivationKind AK, TextWindow *TK, char C) {
		if (!mReady) {
			return;
		}

		if ((mTrigger->getActivationKind() & AK) || mTrigger->requiresStreaming()) {
			mTrigger->activate(TK, C);
			if (!mTrigger->requiresStreaming()) {
				reset();
			}
		}

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
	void processInput(TextWindow *TW, char Char) {
		// Space resets every Trigger
		if (Char == ' ') {
			for (auto && Candidate : mCandidates) {
				Candidate.reset();
			}
			return;
		}

		if (Char == '\t' || Char == '\n') {
			Trigger::ActivationKind AK;
			switch (Char) {
			case '\t': AK = Trigger::ActivationKind::Tab;   break;
			case '\n': AK = Trigger::ActivationKind::Enter; break;
			}

			for (auto &&Candidate : mCandidates) {
				Candidate.activate(AK, TW, Char);
				Candidate.reset();
			}

			return;
		}
		
		// TODO: Optimization opportunity : 
		//	we only need to iterate over the "good" candidates.
		//	Consider using std::partition and only going through the good parts
		for (auto &&Candidate : mCandidates) {
			Candidate.eval(Char);
		}


		// Run the immediate triggers
		for (auto &&Candidate : mCandidates) {
			Candidate.activate(Trigger::ActivationKind::Immediate, TW, Char);
		}



	}
private:
	std::vector<Trigger> mTriggers;
	std::vector<TriggerOpt> mCandidates;

};