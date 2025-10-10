#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#if !defined _WIN32 && !defined _WIN64
#include <algorithm>
#endif
#include <random>
#include <thread>
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif
#ifndef EOF
#define EOF (-1)
#endif
#ifndef UNREFERENCED_PARAMETER
#if defined _WIN32 || defined _WIN64
#define UNREFERENCED_PARAMETER(P) (P)
#else
#define UNREFERENCED_PARAMETER(P)
#endif
#endif
using namespace std;
typedef unsigned char HelpKey;
typedef unsigned short Sorting;
typedef unsigned char Value;
typedef unsigned char Point;
typedef unsigned char Player;
typedef unsigned char Count;
typedef long long int Amount;
constexpr long long int TIME_FOR_SLEEP = 3;
constexpr Point JOKER_POINT = 0;
constexpr Player INVALID_PLAYER = (Player)(-1);


enum class Suit : unsigned char
{
	Diamond = 0, 
	Club = 1, 
	Heart = 2, 
	Spade = 3, 
	Black = 4, 
	Red = 5, 
	Cover = 6
};

enum class Type : unsigned char
{
	Empty = 0b00000000, 
	
	Single = 0b00010000, 
	SingleStraight = 0b00010001, 
	SingleFlush = 0b00010010, 
	SingleFlushStraight = 0b00010011, 
	
	Pair = 0b00100000, 
	PairStraight = 0b00100001, 
	PairStraightWithSingle = 0b00100010, 
	PairJokers = 0b00100011, 
	
	Triple = 0b00110000, 
	TripleWithSingle = 0b00110001, 
	TripleWithPair = 0b00110010, 
	TripleWithPairSingle = 0b00110011, 
	TripleStraight = 0b00110100, 
	TripleStraightWithSingle = 0b00110101, 
	TripleStraightWithSingles = 0b00110110, 
	TripleStraightWithPairs = 0b00110111, 
	
	Quadruple = 0b01000000, 
	QuadrupleWithSingle = 0b01000001, 
	QuadrupleWithSingleSingle = 0b01000010, 
	QuadrupleWithPairPair = 0b01000011, 
	QuadrupleStraight = 0b01000100, 
	QuadrupleStraightWithSingle = 0b01000101, 
	QuadrupleJokers = 0b01000110, 
	
	Quintuple = 0b01010000, 
	Sextuple = 0b01100000, 
	Septuple = 0b01110000, 
	Octuple = 0b10000000, 
	
	Invalid = 0b11111111
};

enum class Status : unsigned char
{
	Ready = 0, 
	Initialized = 1, 
	Dealt = 2, 
	Assigned = 3, 
	Started = 4, 
	Over = 5
};

enum class Comparison : char
{
	Smaller = -1, 
	Equal = 0, 
	Greater = 1
};

enum class Score : unsigned char
{
	None = 0, 
	One = 1, 
	Two = 2, 
	Three = 3
};

enum class Action : unsigned char
{
	None = 0, 
	Again = 1, 
	Return = 2, 
	Exit = 3
};


struct Card
{
	Point point = JOKER_POINT; // JOKER_POINT (0) is for the Jokers, the Cover Card, and the default value. 
	Suit suit = Suit::Cover;
	
	Card() : point(JOKER_POINT), suit(Suit::Cover) {}
	Card(const Point point) : point(0 <= point && point <= 13 ? point : JOKER_POINT), suit(Suit::Cover) {}
	Card(const Point point, const Suit suit) : point(0 <= point && point <= 13 ? point : JOKER_POINT), suit(((1 <= point && point <= 13 && (suit <= Suit::Spade || Suit::Cover == suit)) || (JOKER_POINT == point && suit >= Suit::Black)) ? suit : Suit::Cover) {}
	friend bool operator==(const Card& a, const Card& b)
	{
		return a.point == b.point && a.suit == b.suit;
	}
	operator const string() const
	{
		string stringBuffer{};
		switch (this->suit)
		{
		case Suit::Diamond:
			stringBuffer += "方块";
			break;
		case Suit::Club:
			stringBuffer += "梅花";
			break;
		case Suit::Heart:
			stringBuffer += "红桃";
			break;
		case Suit::Spade:
			stringBuffer += "黑桃";
			break;
		case Suit::Black:
			stringBuffer += "小";
			break;
		case Suit::Red:
			stringBuffer += "大";
			break;
		default:
			return "广告牌";
		}
		switch (this->point)
		{
		case 1:
			stringBuffer += "A";
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			stringBuffer += this->point + '0';
			break;
		case 10:
			stringBuffer += "10";
			break;
		case 11:
			stringBuffer += "J";
			break;
		case 12:
			stringBuffer += "Q";
			break;
		case 13:
			stringBuffer += "K";
			break;
		case JOKER_POINT:
			stringBuffer += "王";
			break;
		default:
			break;
		}
		return stringBuffer;
	}
};

struct Hand
{
	Player player = INVALID_PLAYER;
	vector<Card> cards{};
	Type type = Type::Invalid;
	
	operator const bool() const
	{
		return this->player != INVALID_PLAYER || this->type != Type::Invalid;
	}
	friend bool operator==(const Hand& a, const Hand& b)
	{
		return a.player == b.player && a.cards == b.cards && a.type == b.type;
	}
};

struct Candidate
{
	Hand hand{};
	string description{};
	
	Candidate(const Hand& t, const string& s) : hand(t), description(s) {}
};


class Values
{
private:
	Value values[14] = { 0 };

public:
	Values()
	{
		
	}
	const bool set(const Point point, const Value value)
	{
		if (0 <= point && point <= 13 && 1 <= value && value <= 14)
		{
			this->values[point] = value;
			return true;
		}
		else
			return false;
	}
	const bool get(const Point point, Value& value) const
	{
		if (0 <= point && point <= 13)
		{
			value = this->values[point];
			return true;
		}
		else
			return false;
	}
	const Value operator[](const Point point) const
	{
		return 0 <= point && point <= 13 ? this->values[point] : 0;
	}
};

class PokerGame
{
protected:
	mt19937 seed{};
	string pokerType = "扑克牌";
	Values values{};
	vector<vector<Card>> players{};
	vector<Card> deck{};
	vector<vector<Hand>> records{};
	Player currentPlayer = INVALID_PLAYER, dealer = INVALID_PLAYER;
	Hand lastHand{};
	vector<Amount> amounts{};
	Status status = Status::Ready;
	
private:
	virtual const void add52CardsToDeck(vector<Card>& _deck) const final
	{
		for (Point point = 1; point <= 13; ++point)
		{
			_deck.push_back(Card{ point, Suit::Diamond });
			_deck.push_back(Card{ point, Suit::Club });
			_deck.push_back(Card{ point, Suit::Heart });
			_deck.push_back(Card{ point, Suit::Spade });
		}
		return;
	}
	virtual const void add54CardsToDeck(vector<Card>& _deck) const final
	{
		this->add52CardsToDeck(_deck);
		_deck.push_back(Card{ JOKER_POINT, Suit::Black });
		_deck.push_back(Card{ JOKER_POINT, Suit::Red });
		return;
	}
	
protected:
	/* PokerGame::deal */
	virtual const void add52CardsToDeck() final
	{
		this->add52CardsToDeck(this->deck);
		return;
	}
	virtual const void add54CardsToDeck() final
	{
		this->add54CardsToDeck(this->deck);
		return;
	}
	virtual const bool sortCards(vector<Card>& cards, const Sorting _sorting) const final
	{
		Sorting sorting = _sorting;
		bool pointFlag = false, valueFlag = false, suitFlag = false, pointCountFlag = false, unionCountFlag = false, valueCountFlag = false, suitCountFlag = false;
		vector<function<const int(const Card, const Card)>> lambdas{};
		Count pointCounts[14] = { 0 }, unionCounts[14][4] = { 0 }, valueCounts[15] = { 0 }, suitCounts[7] = { 0 };
		while (sorting)
		{
			switch (sorting & 0b1111)
			{
			case 0b0000: // 'P' (0b01010000)
				if (pointFlag)
					return false;
				else
				{
					pointFlag = true;
					lambdas.emplace_back([](const Card a, const Card b) { const Point pointA = a.point, pointB = b.point; return pointA > pointB ? -1 : pointA < pointB; });
					break;
				}
			case 0b0010: // 'V' (0b01010110)
				if (valueFlag)
					return false;
				else
				{
					valueFlag = true;
					lambdas.emplace_back([this](const Card a, const Card b) { const Value valueA = this->values[a.point], valueB = this->values[b.point]; return valueA > valueB ? -1 : valueA < valueB; });
					break;
				}
			case 0b0011: // 'S' (0b01010011)
				if (suitFlag)
					return false;
				else
				{
					suitFlag = true;
					lambdas.emplace_back([](const Card a, const Card b) { const Suit suitA = a.suit, suitB = b.suit; return suitA > suitB ? -1 : suitA < suitB; });
					break;
				}
			case 0b0100: // 'H' (0b01001000) = 'P' (0b01010000) - 0b1000
				if (pointCountFlag)
					return false;
				else
				{
					pointCountFlag = true;
					lambdas.emplace_back([&pointCounts](const Card a, const Card b) { const Count countA = pointCounts[a.point], countB = pointCounts[b.point]; return countA > countB ? -1 : countA < countB; });
					break;
				}
			case 0b0101: // 'M' (0b01001101) = 'U' (0b01010101) - 0b1000
				if (unionCountFlag)
					return false;
				else
				{
					unionCountFlag = true;
					lambdas.emplace_back([&unionCounts](const Card a, const Card b) { const Count countA = unionCounts[a.point][static_cast<unsigned char>(a.suit) & 0b11], countB = unionCounts[b.point][static_cast<unsigned char>(b.suit) & 0b11]; return countA > countB ? -1 : countA < countB; });
					break;
				}
			case 0b0110: // 'N' (0b01001110) = 'V' (0b01010110) - 0b1000
				if (valueCountFlag)
					return false;
				else
				{
					valueCountFlag = true;
					lambdas.emplace_back([&valueCounts,this](const Card a, const Card b) { const Count countA = valueCounts[this->values[a.point]], countB = valueCounts[this->values[b.point]]; return countA > countB ? -1 : countA < countB; });
					break;
				}
			case 0b0111: // 'K' (0b01001011) = 'S' (0b01010011) - 0b1000
				if (suitCountFlag)
					return false;
				else
				{
					suitCountFlag = true;
					lambdas.emplace_back([&suitCounts](const Card a, const Card b) { const Count countA = suitCounts[static_cast<unsigned char>(a.suit)], countB = suitCounts[static_cast<unsigned char>(b.suit)]; return countA > countB ? -1 : countA < countB; });
					break;
				}
			case 0b1000: // 'p' (0b01110000)
				if (pointFlag)
					return false;
				else
				{
					pointFlag = true;
					lambdas.emplace_back([](const Card a, const Card b) { const Point pointA = a.point, pointB = b.point; return pointA < pointB ? -1 : pointA > pointB; });
					break;
				}
			case 0b1010: // 'v' (0b01110110)
				if (valueFlag)
					return false;
				else
				{
					valueFlag = true;
					lambdas.emplace_back([this](const Card a, const Card b) { const Value valueA = this->values[a.point], valueB = this->values[b.point]; return valueA < valueB ? -1 : valueA > valueB; });
					break;
				}
			case 0b1011: // 's' (0b01110011)
				if (suitFlag)
					return false;
				else
				{
					suitFlag = true;
					lambdas.emplace_back([](const Card a, const Card b) { const Suit suitA = a.suit, suitB = b.suit; return suitA < suitB ? -1 : suitA > suitB; });
					break;
				}
			case 0b1100: // 'h' (0b01101000) = 'p' (0b01110000) - 0b1000
				if (pointCountFlag)
					return false;
				else
				{
					pointCountFlag = true;
					lambdas.emplace_back([&pointCounts](const Card a, const Card b) { const Count countA = pointCounts[a.point], countB = pointCounts[b.point]; return countA < countB ? -1 : countA > countB; });
					break;
				}
			case 0b1101: // 'm' (0b01101101) = 'u' (0b01110101) - 0b1000
				if (unionCountFlag)
					return false;
				else
				{
					unionCountFlag = true;
					lambdas.emplace_back([&unionCounts](const Card a, const Card b) { const Count countA = unionCounts[a.point][static_cast<unsigned char>(a.suit) & 0b11], countB = unionCounts[b.point][static_cast<unsigned char>(b.suit) & 0b11]; return countA < countB ? -1 : countA > countB; });
					break;
				}
			case 0b1110: // 'n' (0b01101110) = 'v' (0b01110110) - 0b1000
				if (valueCountFlag)
					return false;
				else
				{
					valueCountFlag = true;
					lambdas.emplace_back([&valueCounts, this](const Card a, const Card b) { const Count countA = valueCounts[this->values[a.point]], countB = valueCounts[this->values[b.point]]; return countA < countB ? -1 : countA > countB; });
					break;
				}
			case 0b1111: // 'k' (0b01101011) = 's' (0b01110011) - 0b1000
				if (suitCountFlag)
					return false;
				else
				{
					suitCountFlag = true;
					lambdas.emplace_back([&suitCounts](const Card a, const Card b) { const Count countA = suitCounts[static_cast<unsigned char>(a.suit)], countB = suitCounts[static_cast<unsigned char>(b.suit)]; return countA < countB ? -1 : countA > countB; });
					break;
				}
			default:
				break;
			}
			sorting >>= 4;
		}
		if (lambdas.empty())
			sort(cards.begin(), cards.end(), [this](const Card a, const Card b) { const Value valueA = this->values[a.point], valueB = this->values[b.point]; return valueA > valueB || (valueA == valueB && a.suit > b.suit); });
		else
		{
			if (pointCountFlag)
				for (const Card& card : cards)
					++pointCounts[card.point];
			if (unionCountFlag)
				for (const Card& card : cards)
					++unionCounts[card.point][static_cast<unsigned char>(card.suit) & 0b11];
			if (valueCountFlag)
				for (const Card& card : cards)
					++valueCounts[this->values[card.point]];
			if (suitCountFlag)
				for (const Card& card : cards)
					++suitCounts[static_cast<unsigned char>(card.suit)];
			sort(cards.begin(), cards.end(), [&lambdas](const Card a, const Card b) { for (const function<const int(const Card, const Card)>& lambda : lambdas) { const int result = lambda(a, b); if (result) return result > 0; } return false; });
		}
		return true;
	}
	virtual const bool sortCards(vector<Card>& cards) const final
	{
		return this->sortCards(cards, 0b10111010);
	}
	virtual const bool assignDealer()
	{
		if (Status::Dealt == this->status && this->records.empty())
		{
			this->records.push_back(vector<Hand>{});
			const size_t playerCount = this->players.size();
			for (Player player = 0; player < playerCount; ++player)
				this->records[0].push_back(Hand{ player, vector<Card>{ this->players[player].back() } });
			sort(this->records[0].begin(), this->records[0].end(), [this](Hand a, Hand b) { return this->values[a.cards.back().point] > this->values[b.cards.back().point] || (this->values[a.cards.back().point] == this->values[b.cards.back().point] && a.cards.back().suit > b.cards.back().suit); });
			this->currentPlayer = this->records[0].back().player;
			this->dealer = this->records[0].back().player;
			this->lastHand = Hand{};
			this->amounts.clear();
			this->status = Status::Assigned;
			return true;
		}
		else
			return false;
	}
	
	/* PokerGame::setLandlord */
	virtual const bool nextPlayer()
	{
		const size_t playerCount = this->players.size();
		if (0 <= this->currentPlayer && this->currentPlayer < playerCount)
		{
			const Player originalPlayer = this->currentPlayer;
			for (;;)
			{
				if (++this->currentPlayer >= playerCount) // This can still work correctly even when ``this->currentPlayer`` is 255. 
					this->currentPlayer = 0;
				if (originalPlayer == this->currentPlayer)
					return false;
				else if (!this->players[this->currentPlayer].empty())
					return true;
			}
		}
		else
			return false;
	}
	
	/* PokerGame::start */
	virtual const bool description2cards(const string& description, vector<Card>& cards) const final
	{
		if ("/" == description || "\\" == description || "-" == description || "--" == description || "要不起" == description || "不出" == description || "不打" == description)
		{
			cards.clear();
			return true;
		}
		else if (0 <= this->currentPlayer && this->currentPlayer < this->players.size() && !this->players[this->currentPlayer].empty())
		{
			vector<size_t> selected{};
			vector<Card> exactCards{};
			vector<Point> fuzzyPoints{};
			vector<Suit> fuzzySuits{};
			bool waitingForAPoint = false;
			Suit suit = Suit::Diamond;
			const size_t descriptionLength = description.length();
			for (size_t idx = 0; idx < descriptionLength; ++idx)
			{
				switch (description.at(idx))
				{
				case 'A':
				case 'a':
					if (waitingForAPoint)
						exactCards.push_back(Card{ 1, suit });
					else
						fuzzyPoints.push_back(1);
					break;
				case '1':
					if (idx + 1 < descriptionLength && '0' == description.at(idx + 1))
					{
						if (waitingForAPoint)
							exactCards.push_back(Card{ 10, suit });
						else
							fuzzyPoints.push_back(10);
						++idx;
					}
					else if (waitingForAPoint)
						exactCards.push_back(Card{ 1, suit });
					else
						fuzzyPoints.push_back(1);
					break;
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					if (waitingForAPoint)
						exactCards.push_back(Card{ (Point)(description.at(idx) - '0'), suit });
					else
						fuzzyPoints.push_back(description.at(idx) - '0');
					break;
				case 'T':
				case 't':
					if (waitingForAPoint)
						exactCards.push_back(Card{ 10, suit });
					else
						fuzzyPoints.push_back(10);
				case 'J':
				case 'j':
					if (waitingForAPoint)
						exactCards.push_back(Card{ 11, suit });
					else
						fuzzyPoints.push_back(11);
					break;
				case 'Q':
				case 'q':
					if (waitingForAPoint)
						exactCards.push_back(Card{ 12, suit });
					else
						fuzzyPoints.push_back(12);
					break;
				case 'K':
				case 'k':
					if (waitingForAPoint)
						exactCards.push_back(Card{ 13, suit });
					else
						fuzzyPoints.push_back(13);
					break;
				case 'L':
				case 'l':
					if (waitingForAPoint)
						fuzzySuits.push_back(suit);
					else
						exactCards.push_back(Card{ JOKER_POINT, Suit::Black });
					break;
				case 'B':
				case 'b':
					if (waitingForAPoint)
						fuzzySuits.push_back(suit);
					else
						exactCards.push_back(Card{ JOKER_POINT, Suit::Black });
					break;
				default:
				{
					if (waitingForAPoint)
					{
						fuzzySuits.push_back(suit);
						waitingForAPoint = false;
					}
					const string str = description.substr(idx, 4);
					if ("方块" == str)
					{
						suit = Suit::Diamond;
						waitingForAPoint = true;
						idx += 3;
					}
					else if ("梅花" == str)
					{
						suit = Suit::Club;
						waitingForAPoint = true;
						idx += 3;
					}
					else if ("红桃" == str || "红心" == str)
					{
						suit = Suit::Heart;
						waitingForAPoint = true;
						idx += 3;
					}
					else if ("黑桃" == str)
					{
						suit = Suit::Spade;
						waitingForAPoint = true;
						idx += 3;
					}
					else if ("小王" == str || "小鬼" == str)
					{
						suit = Suit::Black;
						exactCards.push_back(Card{ JOKER_POINT, Suit::Black });
						idx += 3;
					}
					else if ("大王" == str || "大鬼" == str)
					{
						suit = Suit::Red;
						exactCards.push_back(Card{ JOKER_POINT, Suit::Red });
						idx += 3;
					}
					break;
				}
				}
			}
			if (waitingForAPoint)
				fuzzySuits.push_back(suit);
			const size_t length = this->players[this->currentPlayer].size();
			size_t position = 0;
			for (const Card& card : exactCards) // select the rightmost one
			{
				bool flag = false;
				for (size_t idx = 0; idx < length; ++idx)
					if (this->values[this->players[this->currentPlayer][idx].point] > this->values[card.point])
						continue;
					else if (this->players[this->currentPlayer][idx] == card && find(selected.begin(), selected.end(), idx) == selected.end())
					{
						position = idx;
						flag = true;
					}
					else
						break;
				if (flag)
					selected.push_back(position);
				else
					return false;
			}
			for (const Point& point : fuzzyPoints) // search for the smallest suit that is not selected for each point to select
			{
				bool flag = false;
				for (size_t idx = 0; idx < length; ++idx)
					if (this->values[this->players[this->currentPlayer][idx].point] > this->values[point])
						continue;
					else if (this->players[this->currentPlayer][idx].point == point)
					{
						if (find(selected.begin(), selected.end(), idx) == selected.end())
						{
							position = idx;
							flag = true;
						}
					}
					else
						break;
				if (flag)
					selected.push_back(position);
				else
					return false;
			}
			for (const Suit& s : fuzzySuits) // select from right to left
			{
				bool flag = false;
				for (size_t idx = length - 1; idx > 0; --idx)
					if (this->players[this->currentPlayer][idx].suit == s && find(selected.begin(), selected.end(), idx) == selected.end())
					{
						position = idx;
						flag = true;
						break;
					}
				if (flag)
					selected.push_back(position);
				else if (this->players[this->currentPlayer][0].suit == s) // avoid (size_t)(-1)
					selected.push_back(0);
				else
					return false;
			}
			cards.clear();
			for (const size_t& p : selected)
				cards.push_back(this->players[this->currentPlayer][p]);
			return true;
		}
		else
			return false;
	}
	virtual const bool checkStarting(const vector<Card>& cards) const
	{
		return !cards.empty() && (this->records[0].back().cards.size() == 1 && find(cards.begin(), cards.end(), this->records[0].back().cards[0]) != cards.end());
	}
	virtual const bool convertPointToChars(const Point point, char buffer[]) const final
	{
		const unsigned long int bufferSize = sizeof(buffer);
		if (1 <= point && point <= 13 && bufferSize >= 3)
		{
			memset(buffer, 0, bufferSize);
			switch (point)
			{
			case 1:
				snprintf(buffer, 2, "A");
				return true;
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				snprintf(buffer, 3, "%d", point);
				return true;
			case 11:
				snprintf(buffer, 2, "J");
				return true;
			case 12:
				snprintf(buffer, 2, "Q");
				return true;
			case 13:
				snprintf(buffer, 2, "K");
				return true;
			default:
				return false;
			}
		}
		else
			return false;
	}
	virtual const bool judgeStraight(vector<Card>& cards, const Count repeatedCount, const Point pointNotAllowedToConnectK, const bool applySorting) const final // This function can only be used when every point is valid with the same count. 
	{
		const size_t cardCount = cards.size();
		if (1 <= repeatedCount && repeatedCount <= 4 && cardCount >= repeatedCount && cardCount % repeatedCount == 0 && 1 <= pointNotAllowedToConnectK && pointNotAllowedToConnectK <= 12)
		{
			vector<Card> sortedCards(cards);
			sort(sortedCards.begin(), sortedCards.end(), [](const Card a, const Card b) { return a.point > b.point || (a.point == b.point && a.suit < b.suit); });
			const size_t indexToLastPoint = cardCount - repeatedCount;
			if (JOKER_POINT == sortedCards[indexToLastPoint].point)
				return false;
			else if (static_cast<size_t>(repeatedCount) * 13 == cardCount) // Straights from the largest to the smallest
			{
				if (applySorting)
				{
					sort(sortedCards.begin(), sortedCards.end(), [this](const Card a, const Card b) { return this->values[a.point] > this->values[b.point] || (a.point == b.point && a.suit > b.suit); });
					cards = sortedCards;
				}
				return true;
			}
			else
			{
				for (size_t idx = 0; idx < indexToLastPoint; ++idx) // Straights like K ... 2 and Q ... A
					if (sortedCards[idx + repeatedCount].point + 1 != sortedCards[idx].point)
						if (sortedCards[idx += repeatedCount].point < pointNotAllowedToConnectK && 13 == sortedCards[0].point && 1 == sortedCards[indexToLastPoint].point)
						{
							const size_t indexToRotation = idx;
							for (; idx < indexToLastPoint; ++idx) // Straights from a point in [1, ``pointNotAllowedToConnectK`` - 1] to a point in [``pointNotAllowedToConnectK`` + 1, 13] and from a point in [1, ``pointNotAllowedToConnectK`` - 2] to a point in [``pointNotAllowedToConnectK``, 13]
								if (sortedCards[idx + 1].point + 1 != sortedCards[idx].point)
									return false;
							if (applySorting)
							{
								rotate(sortedCards.begin(), sortedCards.begin() + indexToRotation, sortedCards.end());
								cards = sortedCards;
							}
							return true;
						}
						else
							return false;
				if (applySorting)
					cards = sortedCards;
				return true;
			}
		}
		else
			return false;
	}
	virtual const Count judgeStraight(vector<Card>& cards, const Point pointNotAllowedToConnectK, const bool applySorting) const final // This function can be used at any time. 
	{
		if (!cards.empty() && 1 <= pointNotAllowedToConnectK && pointNotAllowedToConnectK <= 12)
		{
			/* Packing */
			vector<Card> sortedCards(cards);
			sort(sortedCards.begin(), sortedCards.end(), [](const Card& a, const Card& b) { return a.point > b.point || (a.point == b.point && a.suit > b.suit); });
			Point lastPoint = JOKER_POINT;
			vector<vector<Card>> blocks{};
			for (const Card& card : sortedCards)
				if (JOKER_POINT == card.point)
					return 0;
				else if (card.point == lastPoint)
					blocks.back().push_back(card);
				else if (this->values[card.point])
				{
					blocks.push_back(vector<Card>{ card });
					lastPoint = card.point;
				}
				else
					return 0;
			
			/* Count computing */
			const size_t indexToLastBlock = blocks.size() - 1;
			const Count repeatedCount = static_cast<Count>(blocks[0].size());
			for (size_t idx = 1; idx <= indexToLastBlock; ++idx)
				if (blocks[idx].size() != repeatedCount)
					return 0;
			
			/* Straight judgement */
			if (12 == indexToLastBlock) // Straights from the largest to the smallest
				return repeatedCount; // No need to rotate
			for (size_t idx = 0; idx < indexToLastBlock; ++idx) // Straights like K ... 2
				if (blocks[idx + 1][0].point + 1 != blocks[idx][0].point)
					if (blocks[++idx][0].point < pointNotAllowedToConnectK && 13 == blocks[0][0].point && 1 == blocks.back()[0].point)
					{
						const size_t indexToRotation = idx;
						for (; idx < indexToLastBlock; ++idx) // Straights from a point in [1, ``pointNotAllowedToConnectK`` - 1] to a point in [``pointNotAllowedToConnectK`` + 1, 13] and from a point in [1, ``pointNotAllowedToConnectK`` - 2] to a point in [``pointNotAllowedToConnectK``, 13]
							if (blocks[idx + 1][0].point + 1 != blocks[idx][0].point)
								return 0;
						if (applySorting)
							rotate(blocks.begin(), blocks.begin() + indexToRotation, blocks.end());
						break;
					}
					else
						return 0;
			if (applySorting)
			{
				size_t idx = 0;
				for (const vector<Card>& item : blocks)
					for (const Card& card : item)
						cards[idx++] = card;
			}
			return repeatedCount;
		}
		else
			return 0;
	}
	virtual const bool processHand(Hand& hand, vector<Candidate>& candidates) const = 0;
	virtual const bool removeCards(const vector<Card>& smallerCards, vector<Card>& largerCards) const final // The vector ``largerCards`` must have been sorted according to the default sorting method. 
	{
		vector<Card> sortedCards(smallerCards);
		this->sortCards(sortedCards);
		const size_t smallerLength = sortedCards.size(), largerLength = largerCards.size();
		if (smallerLength > largerLength || 0 == largerLength)
			return false;
		else if (smallerLength >= 1)
		{
			vector<size_t> selected{};
			for (size_t smallerIndex = 0, largerIndex = 0; smallerIndex < smallerLength && largerIndex < largerLength; ++largerIndex)
				if (sortedCards[smallerIndex] == largerCards[largerIndex])
				{
					selected.push_back(largerIndex);
					++smallerIndex;
				}
			if (selected.size() == smallerLength)
			{
				for (size_t idx = smallerLength - 1; idx > 0; --idx)
					largerCards.erase(largerCards.begin() + selected[idx]);
				largerCards.erase(largerCards.begin() + selected[0]);
				return true;
			}
			else
				return false;
		}
		else
			return true;
	}
	virtual const bool processBasis(const Hand& hand) { UNREFERENCED_PARAMETER(hand); return false; }
	virtual const bool isOver() const
	{
		if (this->status >= Status::Started)
			for (const vector<Card>& cards : this->players)
				if (cards.empty())
					return true;
		return false;
	}
	virtual const bool computeAmounts(const unsigned char multiplication1Opening7, const unsigned int basis12Calling4Robbing4Real4Empty4Spring4) { UNREFERENCED_PARAMETER(multiplication1Opening7); UNREFERENCED_PARAMETER(basis12Calling4Robbing4Real4Empty4Spring4); return false; }
	virtual const bool computeAmounts() = 0;
	virtual const bool isAbsolutelyLargest(const Hand& hand) const = 0;
	
	/* PokerGame::play */
	virtual const bool coverLastHand(const Hand& currentHand) const = 0;
	
	/* PokerGame::display */
	virtual const string getBasisString() const { return ""; }
	virtual const string cards2string(const vector<Card>& cards, const string& prefix, const string& separator, const string& suffix, const string& returnIfEmpty) const final
	{
		if (cards.empty())
			return returnIfEmpty;
		else
		{
			string stringBuffer = prefix + (string)cards[0];
			size_t length = cards.size();
			for (size_t cardID = 1; cardID < length; ++cardID)
				stringBuffer += separator + (string)cards[cardID];
			stringBuffer += suffix;
			return stringBuffer;
		}
	}
	virtual const string getPreRoundString() const = 0;
	virtual const string getAmountString() const final
	{
		if (Status::Over == this->status && this->amounts.size() == this->players.size())
		{
			string amountString = "/* 结算信息 */\n";
			char playerBuffer[4] = { 0 }, amountBuffer[21] = { 0 };
			const size_t playerCount = this->players.size();
			for (Player player = 0; player < playerCount; ++player)
			{
				snprintf(playerBuffer, 4, "%d", player + 1);
				snprintf(amountBuffer, 21, "%lld", this->amounts[player]);
				amountString += (string)"玩家 " + playerBuffer + "：" + amountBuffer + "\n";
			}
			return amountString;
		}
		else
			return "结算信息异常，请各位玩家自行计算结算信息。\n";
	}
	virtual const bool display(const vector<Player>& selectedPlayers, const string& dealerRemark, const string& deckDescription) const final
	{
		switch (this->status)
		{
		case Status::Ready:
			cout << "牌局未初始化，请先初始化牌局。" << endl << endl;
			return true;
		case Status::Initialized:
			cout << "当前牌局（" << this->pokerType << "）已初始化，但暂未开局，请发牌或录入残局数据。" << endl << endl;
			return true;
		case Status::Dealt:
		case Status::Assigned:
		case Status::Started:
		case Status::Over:
		{
			/* Beginning */
			bool flag = true;
			cout << "扑克牌类型：" << this->pokerType << "（";
			switch (this->status)
			{
			case Status::Dealt:
				cout << "已发牌";
				break;
			case Status::Assigned:
				cout << "等待开牌";
				break;
			case Status::Started:
				cout << "正在游戏";
				break;
			case Status::Over:
				cout << "已结束";
				break;
			default:
				break;
			}
			cout << "）" << endl << this->getBasisString() << endl;

			/* Players */
			cout << "/* 玩家区域 */" << endl;
			const size_t playerCount = this->players.size();
			if (Status::Over == this->status)
				for (Player player = 0; player < playerCount; ++player)
					cout << "玩家 " << (player + 1) << (this->dealer == player ? "（" + dealerRemark + "）剩余 " : " 剩余 ") << this->players[player].size() << " 张扑克牌：" << this->cards2string(this->players[player], "\n", " | ", "", "（空）") << endl << endl;
			else
				for (Player player = 0; player < playerCount; ++player)
					if (find(selectedPlayers.begin(), selectedPlayers.end(), player) == selectedPlayers.end())
						cout << "玩家 " << (player + 1) << (this->dealer == player ? "（" + dealerRemark + "）剩余 " : " 剩余 ") << this->players[player].size() << " 张扑克牌：（不可见）" << endl << endl;
					else
						cout << "玩家 " << (player + 1) << (this->dealer == player ? "（" + dealerRemark + "）剩余 " : " 剩余 ") << this->players[player].size() << " 张扑克牌：" << this->cards2string(this->players[player], "\n", " | ", "", "（空）") << endl << endl;
			cout << deckDescription;

			/* Records */
			if (!this->records.empty())
			{
				cout << "/* 出牌记录 */" << endl;
				cout << "预备回合：" << this->getPreRoundString() << endl;
				const size_t roundCount = this->records.size();
				for (size_t round = 1; round < roundCount; ++round)
				{
					cout << "第 " << round << " 回合：";
					if (this->records[round].empty())
						cout << "（无）" << endl;
					else
					{
						cout << this->cards2string(this->records[round][0].cards, "", "+", "", "要不起") << "（玩家 " << (this->records[round][0].player + 1) << "）";
						const size_t handCount = this->records[round].size();
						for (size_t handID = 1; handID < handCount; ++handID)
							cout << " -> " << this->cards2string(this->records[round][handID].cards, "", "+", "", "要不起") << "（玩家 " << (this->records[round][handID].player + 1) << "）";
						cout << endl;
					}
				}
				cout << endl;

				/* Amounts */
				if (Status::Over == this->status)
					cout << this->getAmountString() << endl;
			}
			return flag;
		}
		default:
			cout << "当前牌局状态未知，无法显示牌局状况。" << endl << endl;
			return false;
		}
	}
	
public:
	PokerGame() // seed, pokerType, and status = Status::Ready
	{
		random_device rd;
		mt19937 g(rd());
		this->seed = g;
	}
	virtual ~PokerGame()
	{
		
	}
	virtual const bool initialize() = 0; // values, players (= vector<vector<Card>>(n)), deck (clear), records (clear), currentPlayer (reset), dealer (reset), lastHand (reset), amounts (clear), and status = Status::Initialized
	virtual const bool initialize(const size_t playerCount) = 0; // values, players (= vector<vector<Card>>(n)), deck (clear), records (clear), currentPlayer (reset), dealer (reset), lastHand (reset), amounts (clear), and status = Status::Initialized
	virtual const bool deal() = 0; // players, deck, records (clear) -> records[0], currentPlayer, dealer, lastHand (reset), amounts (clear) | amounts = vector<Amount>{ 0 }, and status = Status::Dealt | Status::Assigned
	virtual const bool getCurrentPlayer(Player& player) const final // const
	{
		if ((Status::Dealt <= this->status && this->status <= Status::Started && 0 <= this->currentPlayer && this->currentPlayer < this->players.size()) || Status::Over == this->status)
		{
			player = this->currentPlayer;
			return true;
		}
		else
			return false;
	}
	virtual const bool setLandlord(const bool b) { UNREFERENCED_PARAMETER(b); return false; } // records[0], currentPlayer, dealer (const) -> dealer, lastHand -> lastHand (reset), amounts[0], and status (const) -> status = Status::Assigned
	virtual const bool setLandlord(const Score score) { UNREFERENCED_PARAMETER(score); return false; } // records[0], currentPlayer, dealer (const) -> dealer, lastHand -> lastHand (reset), amounts[0], and status (const) -> status = Status::Assigned
	virtual const bool start(const vector<Card>& cards, vector<Candidate>& candidates) final // records[1], currentPlayer, lastHand, amounts (const) | amounts[0] | amounts(this->players.size()), and status = Status::Started | Status::Over
	{
		if (Status::Assigned == this->status && this->records.size() == 1 && !this->records[0].empty() && 0 <= this->currentPlayer && this->currentPlayer < this->players.size() && this->checkStarting(cards))
		{
			Hand hand{ this->currentPlayer, cards };
			if (this->processHand(hand, candidates) && this->removeCards(cards, this->players[this->currentPlayer]))
			{
				this->records.push_back(vector<Hand>{ hand });
				this->processBasis(hand);
				if (this->isOver())
				{
					this->currentPlayer = INVALID_PLAYER;
					this->lastHand = Hand{};
					this->status = Status::Over;
					this->computeAmounts();
				}
				else
				{
					if (!this->isAbsolutelyLargest(hand))
						this->nextPlayer();
					this->lastHand = this->records[1][0];
					this->status = Status::Started;
				}
				return true;
			}
			else
				return false;
		}
		else
			return false;

	}
	virtual const bool start(const string& description, vector<Candidate>& candidates) final // records[1], currentPlayer, lastHand, amounts (const) | amounts[0] | amounts(this->players.size()), and status = Status::Started | Status::Over
	{
		if (Status::Assigned == this->status && this->records.size() == 1 && !this->records[0].empty())
		{
			vector<Card> cards{};
			return this->description2cards(description, cards) && this->start(cards, candidates);
		}
		else
			return false;
	}
	virtual const bool play(const vector<Card>& cards, vector<Candidate>& candidates) final // records, currentPlayer, lastHand, amounts (const) | amounts[0] | amounts(this->players.size()), and status (const) -> status = Status::Over
	{
		if (Status::Started == this->status && this->records.size() >= 2 && !this->records.back().empty() && 0 <= this->currentPlayer && this->currentPlayer < this->players.size() && this->lastHand)
		{
			Hand hand{ this->currentPlayer, cards };
			if (this->processHand(hand, candidates))
				if (hand.player == this->lastHand.player)
					if (Type::Empty != hand.type && this->removeCards(cards, this->players[this->currentPlayer]))
						if (this->coverLastHand(hand))
							this->records.back().push_back(hand);
						else
							this->records.push_back(vector<Hand>{ hand });
					else
						return false;
				else if (Type::Empty == hand.type)
				{
					this->records.back().push_back(hand);
					this->nextPlayer();
					return true;
				}
				else if (this->coverLastHand(hand) && this->removeCards(hand.cards, this->players[this->currentPlayer]))
					this->records.back().push_back(hand);
				else
					return false;
			else
				return false;
			this->processBasis(hand);
			if (this->isOver())
			{
				this->currentPlayer = INVALID_PLAYER;
				this->lastHand = Hand{};
				this->status = Status::Over;
				this->computeAmounts();
			}
			else
			{
				if (!this->isAbsolutelyLargest(hand))
					this->nextPlayer();
				this->lastHand = this->records.back().back();
			}
			return true;
		}
		else
			return false;
	}
	virtual const bool play(const string& description, vector<Candidate>& candidates) final // records, currentPlayer, lastHand, amounts (const) | amounts[0] | amounts(this->players.size()), and status (const) -> status = Status::Over
	{
		if (Status::Started == this->status && this->records.size() >= 2 && !this->records.back().empty())
		{
			vector<Card> cards{};
			return this->description2cards(description, cards) && this->play(cards, candidates);
		}
		else
			return false;
	}
	virtual const bool set(const vector<char>& binaryChars) final // values, players, deck, records, currentPlayer, dealer, lastHand, amounts, and status
	{
		const size_t length = binaryChars.size(), playerCount = this->players.size();
		char keyChar = 0;
		vector<char> valueBuffer{};
		vector<Card> cards{};
		for (size_t idx = 0; idx < length; ++idx)
		{
			switch (keyChar)
			{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				const Player player = keyChar & 0b1111;
				if (0 <= player && player < playerCount)
					break;
				else
					return false;
			}
			case 'D':
			case 'd':
				if (this->deal())
					break;
				else
					return false;
			case 'A':
			case 'a':
				if (valueBuffer.size() >= 1)
					if ('0' == valueBuffer[0])
						if (this->setLandlord(false))
							break;
						else
							return false;
					else if (this->setLandlord(true))
						break;
					else
						return false;
				else
					return false;
			case 'L':
			case 'l':
				if (valueBuffer.size() >= 1)
				{
					Score score = Score::None;
					switch (valueBuffer[0])
					{
					case '1':
						score = Score::One;
						break;
					case '2':
						score = Score::Two;
						break;
					case '3':
						score = Score::Three;
						break;
					default:
						break;
					}
					if (this->setLandlord(score))
						break;
					else
						return false;
				}
				else
					return false;
			case 'S':
			case 's':
			{
				vector<Candidate> candidates{};
				if (this->start(cards, candidates))
					break;
				else
					return false;
			}
			case 'P':
			case 'p':
			{
				vector<Candidate> candidates{};
				if (this->play(cards, candidates))
					break;
				else
					return false;
			}
			}
		}
		return true;
	}
	virtual const bool display(const vector<Player>& selectedPlayers) const = 0; // const
	virtual const bool display() const final // const
	{
		const size_t playerCount = this->players.size();
		vector<Player> selectedPlayers(playerCount);
		for (Player player = 0; player < playerCount; ++player)
			selectedPlayers[player] = player;
		return this->display(selectedPlayers);
	}
	virtual const bool display(const Player player) const final { return INVALID_PLAYER == player ? this->display() : this->display(vector<Player>{ player }); } // const
};

class Landlords : public PokerGame /* Next: LandlordsX */
{
private:
	const bool assignDealer() override final
	{
		if (Status::Dealt == this->status && this->records.empty())
		{
			this->records.push_back(vector<Hand>{});
			uniform_int_distribution<size_t> distribution(0, this->players.size() - 1);
			this->currentPlayer = (Player)(distribution(this->seed));
			this->dealer = INVALID_PLAYER;
			this->lastHand = Hand{};
			this->amounts = vector<Amount>{ 0b0 };
			return true;
		}
		else
			return false;
	}
	const bool checkStarting(const vector<Card>& cards) const override final
	{
		return !cards.empty();
	}
	const bool processBasis(const Hand& hand) override final
	{
		if (Status::Assigned <= this->status && this->status <= Status::Started && !this->records.empty() && !this->records.back().empty() && this->amounts.size() == 1)
		{
			if (Type::Quadruple == hand.type || Type::PairJokers == hand.type)
				this->amounts[0] += hand.player == this->lastHand.player ? 0b1 : 0b10000;
			return true;
		}
		else
			return false;
	}
	const bool computeAmounts(const unsigned char multiplication1Opening7, const unsigned int basis12Calling4Robbing4Real4Empty4Spring4) override final
	{
		if (Status::Over == this->status && this->players.size() == 3)
		{
			switch (this->amounts.size())
			{
			case 1: // at most 5 ** 3 * 4096 * 5 ** 18 * 2 = 3906250000000000000 < 1 << 63 = 9223372036854775808
			{
				if (this->amounts[0] < 0)
					return false;
				const Amount backup = this->amounts[0];
				Amount mutableAmounts[5] = { /* calling = */ (backup >> 10) & 0b1, /* robbing = */ (backup >> 8) & 0b11, /* realBooms = */ (backup >> 4) & 0b1111, /* emptyBooms = */ backup & 0b1111 };
				const Amount constAmounts[5] = { mutableAmounts[0], mutableAmounts[1], mutableAmounts[2], mutableAmounts[3] };
				Amount* p = mutableAmounts;
				const Amount* q = constAmounts;
				for (Count count = 0, offset = 16; count < 4; ++count)
				{
					switch ((basis12Calling4Robbing4Real4Empty4Spring4 >> offset) & 0b1111)
					{
					case 0:
						*p = 0;
						break;
					case 1:
						break;
					case 2:
						*p <<= 1;
						break;
					case 3:
						*p *= 3;
						break;
					case 4:
						*p <<= 2;
						break;
					case 5:
						*p *= 5;
						break;
					case 6:
						*p *= 6;
						break;
					case 7:
						*p *= 7;
						break;
					case 8:
						*p <<= 3;
						break;
					case 9:
						*p *= 9;
						break;
					case 10:
						*p = 1;
						break;
					case 15:
						*p *= *q;
					case 14:
						*p *= *q;
					case 13:
						*p *= *q;
					case 12:
						*p *= *q;
					case 11:
						break;
					default:
						return false;
					}
					++p;
					++q;
					offset -= 4;
				}
				if (multiplication1Opening7 >> 7)
					this->amounts[0] = static_cast<Amount>(multiplication1Opening7 & 0b1111111) * (basis12Calling4Robbing4Real4Empty4Spring4 >> 20) * mutableAmounts[0] * mutableAmounts[1] * mutableAmounts[2] * mutableAmounts[3] * mutableAmounts[4];
				else
					this->amounts[0] = static_cast<Amount>(multiplication1Opening7 & 0b1111111) + (basis12Calling4Robbing4Real4Empty4Spring4 >> 20) + mutableAmounts[0] + mutableAmounts[1] + mutableAmounts[2] + mutableAmounts[3] + mutableAmounts[4];
				this->amounts = vector<Amount>(3);
				Amount s = 0;
				for (Player player = 0; player < 3; ++player)
				{
					this->amounts[player] = player == this->dealer ? (this->players[player].empty() ? backup << 1 : -(backup << 1)) : (this->players[player].empty() ? backup : -backup);
					s += this->amounts[player];
				}
				if (s)
				{
					this->amounts = vector<Amount>{ backup };
					return false;
				}
			}
			case 3:
				return true;
			default:
				return false;
			}
		}
		else
			return false;
	}
	const bool computeAmounts() override final
	{
		return this->computeAmounts(0b10000000, 0b00000000101011001100110011001100);
	}
	const bool isAbsolutelyLargest(const Hand& hand) const override final
	{
		return Type::PairJokers == hand.type;
	}
	const string getBasisString() const
	{
		if (this->amounts.size() == 1)
		{
			char buffers[4][21] = { { 0 } };
			snprintf(buffers[0], 21, "%lld", this->amounts[0] >> 10);
			snprintf(buffers[1], 21, "%lld", this->amounts[0] >> 8);
			snprintf(buffers[2], 21, "%lld", (this->amounts[0] >> 4) & 0b1111);
			snprintf(buffers[3], 21, "%lld", this->amounts[0] & 0b1111);
			return (string)"倍数信息：当前共叫地主 " + buffers[0] + " 次，抢地主 " + buffers[1] + " 次；共出实炸 " + buffers[2] + " 个，空炸 " + buffers[3] + " 个。\n";
		}
		else
			return "";
	}
	const string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			Count callerAndRobberCount = 0;
			const size_t length = this->records[0].size();
			for (size_t idx = 0; idx < length; ++idx)
				if (!this->records[0][idx].cards.empty())
					++callerAndRobberCount;
			char playerBuffer[4] = { 0 };
			if (0 == callerAndRobberCount && length >= 3)
			{
				snprintf(playerBuffer, 4, "%d", (this->records[0][0].player + 1));
				return "无人叫地主，强制玩家 " + (string)playerBuffer + " 为地主。";
			}
			else
			{
				string preRoundString{};
				bool isRobbing = false;
				for (size_t idx = 0; idx < length; ++idx)
				{
					snprintf(playerBuffer, 4, "%d", this->records[0][idx].player + 1);
					if (this->records[0][idx].cards.empty())
						preRoundString += (isRobbing ? "不抢（玩家 " : "不叫（玩家 ") + (string)playerBuffer + "） -> ";
					else if (isRobbing)
						preRoundString += "抢地主（玩家 " + (string)playerBuffer + "） -> ";
					else
					{
						preRoundString += "叫地主（玩家 " + (string)playerBuffer + "） -> ";
						isRobbing = true;
					}
				}
				preRoundString.erase(preRoundString.length() - 4, 4);
				return preRoundString;
			}
		}
	}
	
protected:
	const bool processHand(Hand& hand, vector<Candidate>& candidates) const override
	{
		hand.type = Type::Invalid;
		candidates.clear();
		bool littleJoker = false, bigJoker = false;
		vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (JOKER_POINT == card.point)
				switch (card.suit)
				{
				case Suit::Black:
					if (littleJoker)
						return false;
					else
						littleJoker = true;
				case Suit::Red:
					if (bigJoker)
						return false;
					else
						bigJoker = true;
				default:
					return false;
				}
			else if (this->values[card.point])
				++counts[card.point];
			else
				return false;
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && valueA > valueB) || (countA == countB && valueA == valueB && a.suit > b.suit); });
		if (adjacent_find(hand.cards.begin(), hand.cards.end()) != hand.cards.end())
			return false;
		sort(counts.begin(), counts.end(), [](const Count a, const Count b) { return a > b; });
		if (counts[0] > 4)
			return false;
		switch (hand.cards.size())
		{
		case 0:
			hand.type = Type::Empty; // 要不起
			return true;
		case 1:
			hand.type = Type::Single; // 单牌
			return true;
		case 2:
			if (2 == counts[0])
			{
				hand.type = JOKER_POINT == hand.cards[0].point ? Type::PairJokers : Type::Pair; // 王炸/火箭 | 对子
				return true;
			}
			else
				return false;
		case 3:
			if (3 == counts[0])
			{
				hand.type = Type::Triple; // 三条
				return true;
			}
			else
				return false;
		case 4:
			switch (counts[0])
			{
			case 4:
				hand.type = Type::Quadruple; // 炸弹
				return true;
			case 3: // if (1 == counts[1])
				hand.type = Type::TripleWithSingle; // 三带一
				return true;
			default:
				return false;
			}
		case 5:
			switch (counts[0])
			{
			case 3: // if (2 == counts[1])
				if (JOKER_POINT == hand.cards[3].point) // 双王不是对子/被带的牌不能含有王炸
					return false;
				else
				{
					hand.type = Type::TripleWithPair; // 三带一对
					return true;
				}
			case 1: // && 1 == counts[1] && 1 == counts[2] && 1 == counts[3] && 1 == counts[4]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[4].point] + 4 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			}
		case 6:
			switch (counts[0])
			{
			case 4:
				if (1 == counts[1] || JOKER_POINT != hand.cards[4].point) // (&& 1 == counts[2] || (&& 2 == counts[2] | 被带的牌不能含有王炸))
				{
					hand.type = Type::QuadrupleWithSingleSingle; // 四带二单
					return true;
				}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[4].point] + 2 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[5]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[5].point] + 5 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 7:
			if (1 == counts[0] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 6 == this->values[hand.cards[0].point]) // && 1 == counts[1] && ... && 1 == counts[6]
			{
				hand.type = Type::SingleStraight; // 顺子
				return true;
			}
			else
				return false;
		case 8:
			switch (counts[0])
			{
			case 4:
				if (2 == counts[1] && 2 == counts[2] && JOKER_POINT != hand.cards[4].point) //  && JOKER_POINT != hand.cards[6].point | 双王不是对子/被带的牌不能含有王炸
				{
					hand.type = Type::QuadrupleWithPairPair; // 四带二对
					return true;
				}
				else
					return false;
			case 3:
				if (3 == counts[1] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[3].point] + 1 == this->values[hand.cards[0].point] && (1 == counts[2] || JOKER_POINT != hand.cards[6].point)) // 被带的牌不能含有王炸 || 1 == counts[3]
				{
					hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
					return true;
				}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 3 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[7]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[7].point] + 7 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 9:
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 3 == counts[2] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 2 == this->values[hand.cards[0].point])
				{
					hand.type = Type::TripleStraight; // 飞机（不带翅膀）
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[8]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[8].point] + 8 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 10:
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 2 == counts[2] && 2 == counts[3] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[3].point] + 1 == this->values[hand.cards[0].point] && JOKER_POINT != hand.cards[6].point) // && JOKER_POINT != hand.cards[8].point | 双王不是对子/被带的牌不能含有王炸
				{
					hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
					return true;
				}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[8].point] + 4 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[9]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[9].point] + 9 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 11:
			if (1 == counts[0] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[10].point] + 10 == this->values[hand.cards[0].point]) // && 1 == counts[1] && ... && 1 == counts[10]
			{
				hand.type = Type::SingleStraight; // 顺子
				return true;
			}
			else
				return false;
		case 12:
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 3 == counts[2])
					switch (counts[3])
					{
					case 3:
						if (this->values[hand.cards[0].point] <= 12)
							if (this->values[hand.cards[9].point] + 3 == this->values[hand.cards[0].point])
							{
								hand.type = Type::TripleStraight; // 飞机（不带翅膀）
								return true;
							}
							else if (this->values[hand.cards[6].point] + 2 == this->values[hand.cards[0].point])
							{
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								return true;
							}
						if (this->values[hand.cards[9].point] + 2 == this->values[hand.cards[3].point])
						{
							rotate(hand.cards.begin(), hand.cards.begin() + 3, hand.cards.end()); // e.g., 222999888777 -> 999888777 + 222
							hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
							return true;
						}
						else
							return false;
					case 2: // && 1 == counts[4]
						if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 2 == this->values[hand.cards[0].point] && JOKER_POINT != hand.cards[9].point) // 被带的牌不能含有王炸
						{
							if (this->values[hand.cards[9].point] < this->values[hand.cards[11].point])
								rotate(hand.cards.begin() + 9, hand.cards.begin() + 11, hand.cards.end()); // e.g., 999888777335 -> 999888777 + 533
							hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
							return true;
						}
						else
							return false;
					case 1: // && 1 == counts[4] && 1 == counts[5]
						if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 2 == this->values[hand.cards[0].point])
						{
							hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
							return true;
						}
						else
							return false;
					default:
						return false;
					}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[10].point] + 5 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[11]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[11].point] + 11 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 14:
			if (2 == counts[0] && 2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[12].point] + 6 == this->values[hand.cards[0].point])
			{
				hand.type = Type::PairStraight; // 连对
				return true;
			}
			else
				return false;
		case 15:
			if (3 == counts[0] && 3 == counts[1] && 3 == counts[2])
				switch (counts[3])
				{
				case 3:
					if (3 == counts[4] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
					{
						hand.type = Type::TripleStraight; // 飞机（不带翅膀）
						return true;
					}
					else
						return false;
				case 2:
					if (2 == counts[4] && 2 == counts[5] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 2 == this->values[hand.cards[0].point] && JOKER_POINT != hand.cards[9].point) // && JOKER_POINT != hand.cards[11].point && JOKER_POINT != hand.cards[13].point | 双王不是对子/被带的牌不能含有王炸
					{
						hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
						return true;
					}
					else
						return false;
				default:
					return false;
				}
			else
				return false;
		case 16:
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 3 == counts[2] && 3 == counts[3])
					switch (counts[4])
					{
					case 3:
						if (this->values[hand.cards[0].point] <= 12)
							if (this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
								return false;
							else if (this->values[hand.cards[9].point] + 3 == this->values[hand.cards[0].point])
							{
								if (this->values[hand.cards[12].point] < this->values[hand.cards[15].point])
									rotate(hand.cards.begin() + 12, hand.cards.begin() + 15, hand.cards.end()); // e.g., AAAKKKQQQJJJ3335 -> AAAKKKQQQJJJ + 5333
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								return true;
							}
						if (this->values[hand.cards[12].point] + 3 == this->values[hand.cards[3].point])
						{
							rotate(hand.cards.begin(), hand.cards.begin() + 3, this->values[hand.cards[0].point] < this->values[hand.cards[15].point] ? hand.cards.end() : hand.cards.end() - 1); // e.g., KKK9998887776662 -> 999888777666 + 2KKK | 222999888777666K -> 999888777666 + 222K
							hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
							return true;
						}
						else
							return false;
					case 2:
						if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[9].point] + 3 == this->values[hand.cards[0].point] && JOKER_POINT != hand.cards[12].point) // 被带的牌不能含有王炸
							if (2 == counts[5]) // && JOKER_POINT != hand.cards[14].point |  被带的牌不能含有王炸
							{
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								return true;
							}
							else // if (1 == counts[5] && 1 == counts[6])
							{
								if (this->values[hand.cards[12].point] < this->values[hand.cards[14].point])
									rotate(hand.cards.begin() + 12, hand.cards.begin() + 14, this->values[hand.cards[12].point] < this->values[hand.cards[15].point] ? hand.cards.end() : hand.cards.end() - 1); // e.g., AAAKKKQQQJJJ3375 -> AAAKKKQQQJJJ + 7533 | AAAKKKQQQJJJ5573 -> AAAKKKQQQJJJ + 7553
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								return true;
							}
						else
							return false;
					case 1: // && 1 == counts[5] && 1 == counts[6] && 1 == counts[7]
						if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[9].point] + 3 == this->values[hand.cards[0].point])
						{
							hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
							return true;
						}
						else
							return false;
					default:
						return false;
					}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && 2 == counts[7] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[14].point] + 7 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 18:
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 3 == counts[2] && 3 == counts[3] && 3 == counts[4] && 3 == counts[5] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[15].point] + 5 == this->values[hand.cards[0].point])
				{
					hand.type = Type::TripleStraight; // 飞机（不带翅膀）
					return true;
				}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && 2 == counts[7] && 2 == counts[8] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[16].point] + 8 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 20:
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 3 == counts[2] && 3 == counts[3])
					switch (counts[4])
					{
					case 3:
						switch (counts[5])
						{
						case 3:
							if (this->values[hand.cards[0].point] <= 12)
								if (this->values[hand.cards[15].point] + 5 == this->values[hand.cards[0].point])
									return false;
								else if (this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
									if (2 == counts[6])
										if (JOKER_POINT != hand.cards[18].point) // 被带的牌不能含有王炸
										{
											if (this->values[hand.cards[15].point] < this->values[hand.cards[18].point])
												rotate(hand.cards.begin() + 15, hand.cards.begin() + 18, hand.cards.end()); // e.g., KKK...99933355 -> KKK...999 + 55333
											hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
											return true;
										}
										else
											return false;
									else // if (1 == counts[7])
									{
										if (this->values[hand.cards[15].point] < this->values[hand.cards[18].point])
											rotate(hand.cards.begin() + 15, hand.cards.begin() + 18, this->values[hand.cards[15].point] < this->values[hand.cards[19].point] ? hand.cards.end() : hand.cards.end() - 1); // e.g., KKK...99933375 -> KKK...999 + 75333 | KKK...99955573 -> KKK...999 + 75553
										hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
										return true;
									}
							if (this->values[hand.cards[15].point] + 4 == this->values[hand.cards[3].point])
								if (2 == counts[6])
									if (JOKER_POINT != hand.cards[18].point) // 被带的牌不能含有王炸
									{
										rotate(hand.cards.begin(), hand.cards.begin() + 3, this->values[hand.cards[0].point] < this->values[hand.cards[18].point] ? hand.cards.end() : hand.cards.end() - 2); // e.g., KKK99988877766655522 -> 999888777666555 + 22KKK | 222999888777666555KK -> 999888777666555 + 222KK
										hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
										return true;
									}
									else
										return false;
								else // if (1 == counts[6] && 1 == counts[7])
								{
									rotate(hand.cards.begin(), hand.cards.begin() + 3, this->values[hand.cards[0].point] < this->values[hand.cards[18].point] ? (this->values[hand.cards[0].point] < this->values[hand.cards[19].point] ? hand.cards.end() : hand.cards.end() - 1) : hand.cards.end() - 2); // e.g., JJJ9998887776665552K -> 999888777666555 + 2KJJJ | KKK9998887776665552J -> 999888777666555 + 2KKKJ | 222999888777666555KJ -> 999888777666555 + 222KJ
									hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									return true;
								}
							else
								return false;
						case 2:
							if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
								if (2 == counts[6]) // && 1 == counts[7]
									if (JOKER_POINT != hand.cards[15].point) // && JOKER_POINT != hand.cards[17].point | 被带的牌不能含有王炸
									{
										const Value value19 = this->values[hand.cards[19].point];
										if (this->values[hand.cards[17].point] < value19)
											rotate(hand.cards.begin() + (this->values[hand.cards[15].point] < value19 ? 15 : 17), hand.cards.begin() + 19, hand.cards.end()); // e.g., KKK...99955337 -> KKK...999 + 75533 | KKK...99977335 -> KKK...999 + 77533
										hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
										return true;
									}
									else
										return false;
								else // if (1 == counts[6] && 1 == counts[7] && 1 == counts[8])
								{
									const Value value15 = this->values[hand.cards[15].point];
									if (value15 < this->values[hand.cards[17].point])
										rotate(hand.cards.begin() + 15, hand.cards.begin() + 17, value15 < this->values[hand.cards[18].point] ? (value15 < this->values[hand.cards[19].point] ? hand.cards.end() : hand.cards.end() - 1) : hand.cards.end() - 2); // e.g., KKK...99933765 -> KKK...999 + 76533 | KKK...99944653 -> KKK...99965443 | KKK...99955643 -> KKK...99965543
									hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									return true;
								}
							else
								return false;
						case 1: // && 1 == counts[6] && 1 == counts[7] && 1 == counts[8] && 1 == counts[9]
							hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
							return true;
						default:
							return false;
						}
					case 2:
						if (2 == counts[5] && 2 == counts[6] && 2 == counts[7] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[9].point] + 3 == this->values[hand.cards[0].point] && JOKER_POINT != hand.cards[12].point) // && JOKER_POINT != hand.cards[14].point && JOKER_POINT != hand.cards[16].point && JOKER_POINT != hand.cards[18].point | 双王不是对子/被带的牌不能含有王炸
						{
							hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
							return true;
						}
						else
							return false;
					default:
						return false;
					}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && 2 == counts[7] && 2 == counts[8] && 2 == counts[9] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[18].point] + 9 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		default:
			return false;
		}
	}
	const bool coverLastHand(const Hand& currentHand) const override final
	{
		if (this->lastHand && Type::Single <= this->lastHand.type && this->lastHand.type <= Type::QuadrupleWithPairPair && !this->lastHand.cards.empty() && Type::Single <= currentHand.type && currentHand.type <= Type::QuadrupleWithPairPair && !currentHand.cards.empty())
			switch (this->lastHand.type)
			{
			case Type::Single: // 单牌
				return Type::PairJokers == currentHand.type || Type::Quadruple == currentHand.type || (Type::Single == currentHand.type && (JOKER_POINT == currentHand.cards[0].point && JOKER_POINT == this->lastHand.cards[0].point ? currentHand.cards[0].suit > this->lastHand.cards[0].suit : this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point]));
			case Type::SingleStraight: // 顺子
			case Type::Pair: // 对子
			case Type::PairStraight: // 连对
			case Type::Triple: // 三条
			case Type::TripleWithSingle: // 三带一
			case Type::TripleWithPair: // 三带一对
			case Type::TripleStraight: // 飞机（不带翅膀）
			case Type::TripleStraightWithSingles: // 飞机带小翼
			case Type::TripleStraightWithPairs: // 飞机带大翼
			case Type::QuadrupleWithSingleSingle: // 四带二单
			case Type::QuadrupleWithPairPair: // 四带二对
				return Type::PairJokers == currentHand.type || Type::Quadruple == currentHand.type || (currentHand.type == this->lastHand.type && currentHand.cards.size() == this->lastHand.cards.size() && this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point]);
			case Type::PairJokers: // 王炸/火箭
				return false;
			case Type::Quadruple: // 炸弹
				return Type::PairJokers == currentHand.type || (Type::Quadruple == currentHand.type && this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point]);
			default:
				return false;
			}
		else
			return false;
	}
	
public:
	Landlords() : PokerGame()
	{
		this->pokerType = "斗地主";
	}
	const bool initialize() override final
	{
		if (this->status >= Status::Ready)
		{
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->values.set(JOKER_POINT, value++);
			this->players = vector<vector<Card>>(3);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->dealer = INVALID_PLAYER;
			this->lastHand = Hand{};
			this->amounts.clear();
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
	const bool initialize(const size_t playerCount) override final { return 3 == playerCount && this->initialize(); }
	const bool deal() override final
	{
		if (this->status >= Status::Initialized)
		{
			this->deck.clear();
			this->add54CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < 3; ++player)
			{
				this->players[player] = vector<Card>(17);
				for (size_t idx = 0; idx < 17; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	const bool setLandlord(const bool b) override final
	{
		if (Status::Dealt == this->status && this->records.size() == 1 && 0 <= this->currentPlayer && this->currentPlayer < this->players.size() && this->amounts.size() == 1)
			switch (this->records[0].size())
			{
			case 0:
				if (b)
				{
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{ Card{} } });
					this->lastHand = this->records[0][0];
					this->amounts[0] = 0b10000000000;
				}
				else
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{} });
				this->nextPlayer();
				return true;
			case 1:
				if (b)
				{
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{ Card{} } });
					if (this->lastHand)
						this->amounts[0] += 0b100000000;
					else
					{
						this->lastHand = this->records[0][1];
						this->amounts[0] = 0b10000000000;
					}
				}
				else
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{} });
				this->nextPlayer();
				return true;
			case 2:
			{
				if (b)
				{
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{ Card{} } });
					if (this->lastHand)
						this->amounts[0] += 0b100000000;
					else
						this->amounts[0] = 0b10000000000;
				}
				else
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{} });
				Count callerAndRobberCount = 0;
				for (size_t idx = 0; idx < 3; ++idx)
					if (!this->records[0][idx].cards.empty())
						++callerAndRobberCount;
				switch (callerAndRobberCount)
				{
				case 0:
					this->currentPlayer = this->records[0][0].player;
					this->dealer = this->records[0][0].player;
					this->lastHand = Hand{};
					this->status = Status::Assigned;
					return true;
				case 1:
					this->currentPlayer = this->lastHand.player;
					this->dealer = this->lastHand.player;
					this->lastHand = Hand{};
					this->status = Status::Assigned;
					return true;
				case 2:
				case 3:
					this->currentPlayer = this->lastHand.player;
					return true;
				default:
					return false;
				}
			}
			case 3:
				if (b)
				{
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{ Card{} } });
					if (this->lastHand)
						this->amounts[0] += 0b100000000;
					else
						return false;
				}
				else
				{
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{} });
					this->currentPlayer = this->records[0][2].cards.empty() ? this->records[0][1].player : this->records[0][2].player;
				}
				this->dealer = this->currentPlayer;
				this->lastHand = Hand{};
				this->status = Status::Assigned;
				return true;
			default:
				return false;
			}
		else
			return false;
	}
	const bool display(const vector<Player>& selectedPlayers) const override final
	{
		return this->status >= Status::Assigned ? PokerGame::display(selectedPlayers, "地主", "地主牌：" + this->cards2string(this->deck, "", " | ", "（已公开）", "（空）") + "\n\n") : PokerGame::display(selectedPlayers, "拥有明牌", "地主牌：" + this->cards2string(this->deck, "", " | ", "（未公开）", "（空）") + "\n\n");
	}
};

class LandlordsX : public Landlords /* Previous: Landlords | Next: Landlord4P */
{
protected:
	const bool processHand(Hand& hand, vector<Candidate>& candidates) const override final
	{
		hand.type = Type::Invalid;
		bool littleJoker = false, bigJoker = false;
		vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (JOKER_POINT == card.point)
				switch (card.suit)
				{
				case Suit::Black:
					if (littleJoker)
					{
						candidates.clear();
						return false;
					}
					else
						littleJoker = true;
				case Suit::Red:
					if (bigJoker)
					{
						candidates.clear();
						return false;
					}
					else
						bigJoker = true;
				default:
					candidates.clear();
					return false;
				}
			else if (this->values[card.point])
				++counts[card.point];
			else
			{
				candidates.clear();
				return false;
			}
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && valueA > valueB) || (countA == countB && valueA == valueB && a.suit > b.suit); });
		if (adjacent_find(hand.cards.begin(), hand.cards.end()) != hand.cards.end())
		{
			candidates.clear();
			return false;
		}
		sort(counts.begin(), counts.end(), [](const Count a, const Count b) { return a > b; });
		if (counts[0] > 4)
		{
			candidates.clear();
			return false;
		}
		switch (hand.cards.size())
		{
		case 0:
			hand.type = Type::Empty; // 要不起
			candidates.clear();
			return true;
		case 1:
			hand.type = Type::Single; // 单牌
			candidates.clear();
			return true;
		case 2:
			candidates.clear();
			if (2 == counts[0])
			{
				hand.type = JOKER_POINT == hand.cards[0].point ? Type::PairJokers : Type::Pair; // 王炸/火箭 | 对子
				return true;
			}
			else
				return false;
		case 3:
			candidates.clear();
			if (3 == counts[0])
			{
				hand.type = Type::Triple; // 三条
				return true;
			}
			else
				return false;
		case 4:
			switch (counts[0])
			{
			case 4:
			{
				vector<Candidate> potentialHands{};
				char buffer[3] = { 0 };
				this->convertPointToChars(hand.cards[0].point, buffer);
				potentialHands.push_back(Candidate{ hand, (string)"解析为点数为 " + buffer + " 的炸弹（``Type::Quadruple``）" });
				potentialHands.push_back(Candidate{ hand, (string)"解析为点数为 " + buffer + " 的三带一（``Type::TripleWithSingle``）" });
				break;
			}
			case 3: // && 1 == counts[1]
				hand.type = Type::TripleWithSingle; // 三带一
				candidates.clear();
				return true;
			default:
				candidates.clear();
				return false;
			}
		case 5:
			candidates.clear();
			switch (counts[0])
			{
			case 3: // if (2 == counts[1])
				if (JOKER_POINT == hand.cards[3].point) // 双王不是对子
					return false;
				else
				{
					hand.type = Type::TripleWithPair; // 三带一对
					return true;
				}
			case 1: // && 1 == counts[1] && 1 == counts[2] && 1 == counts[3] && 1 == counts[4]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[4].point] + 4 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			}
		case 6:
			candidates.clear();
			switch (counts[0])
			{
			case 4:
				hand.type = Type::QuadrupleWithSingleSingle; // 四带二单
				return true;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[4].point] + 2 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[5]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[5].point] + 5 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 7:
			candidates.clear();
			if (1 == counts[0] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 6 == this->values[hand.cards[0].point]) // && 1 == counts[1] && ... && 1 == counts[6]
			{
				hand.type = Type::SingleStraight; // 顺子
				return true;
			}
			else
				return false;
		case 8:
			switch (counts[0])
			{
			case 4:
				switch (counts[1])
				{
				case 4:
				{
					const Value value = this->values[hand.cards[0].point];
					vector<Candidate> potentialHands{};
					char buffers[2][3] = { 0 };
					this->convertPointToChars(hand.cards[0].point, buffers[0]);
					this->convertPointToChars(hand.cards[4].point, buffers[1]);
					if (value <= 12 && this->values[hand.cards[4].point] + 1 == value)
					{
						potentialHands.emplace_back(hand, (string)"解析为长度为 2 且点数为 " + buffers[0] + " 的飞机带小翼（``Type::TripleStraightWithSingles``）");
						rotate(potentialHands.back().hand.cards.begin() + 3, potentialHands.back().hand.cards.begin() + 4, potentialHands.back().hand.cards.begin() + 7);
						potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
					}
					potentialHands.emplace_back(hand, (string)"解析为点数为 " + buffers[0] + " 的四带二对（``Type::QuadrupleWithPairPair``），其中被带的牌包含两个点数为 " + buffers[1] + " 的对子");
					potentialHands.back().hand.type = Type::QuadrupleWithPairPair; // 四带二对
					potentialHands.emplace_back(hand, (string)"解析为点数为 " + buffers[1] + " 的四带二对（``Type::QuadrupleWithPairPair``），其中被带的牌包含两个点数为 " + buffers[0] + " 的对子");
					rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 4, potentialHands.back().hand.cards.end());
					potentialHands.back().hand.type = Type::QuadrupleWithPairPair; // 四带二对
					if (this->lastHand && hand.player != this->lastHand.player)
						for (vector<Candidate>::iterator it = potentialHands.begin(); it != potentialHands.end();)
							if (this->coverLastHand(it->hand))
								++it;
							else
								it = potentialHands.erase(it);
					switch (potentialHands.size())
					{
					case 0:
						candidates.clear();
						return false;
					case 1:
						hand = move(potentialHands[0].hand);
						candidates.clear();
						return true;
					default:
						if (candidates.size() == 1)
						{
							const vector<Candidate>::iterator it = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
							if (it != potentialHands.end())
							{
								hand = move(it->hand);
								candidates.clear();
								return true;
							}
						}
						candidates = move(potentialHands);
						return false;
					}
				}
				case 3: // && 1 == counts[2]
					candidates.clear();
					if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[4].point] + 1 == this->values[hand.cards[0].point])
					{
						rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.begin() + 7);
						hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
						return true;
					}
					else
						return false;
				case 2:
					candidates.clear();
					if (2 == counts[2] && JOKER_POINT != hand.cards[4].point) //  && JOKER_POINT != hand.cards[6].point | 双王不是对子
					{
						hand.type = Type::QuadrupleWithPairPair; // 四带二对
						return true;
					}
					else
						return false;
				default:
					candidates.clear();
					return false;
				}
			case 3:
				candidates.clear();
				if (3 == counts[1] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[3].point] + 1 == this->values[hand.cards[0].point])
				{
					hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
					return true;
				}
				else
					return false;
			case 2:
				candidates.clear();
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 3 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[7]
				candidates.clear();
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[7].point] + 7 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				candidates.clear();
				return false;
			}
		case 9:
			candidates.clear();
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 3 == counts[2] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 2 == this->values[hand.cards[0].point])
				{
					hand.type = Type::TripleStraight; // 飞机（不带翅膀）
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[8]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[8].point] + 8 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 10:
			candidates.clear();
			switch (counts[0])
			{
			case 4:
				if (3 == counts[1] && 3 == counts[2] && this->values[hand.cards[7].point] + 1 == this->values[hand.cards[4].point])
				{
					rotate(hand.cards.begin(), hand.cards.begin() + 4, hand.cards.end());
					hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
					return true;
				}
				else
					return false;
			case 3:
				if (3 == counts[1] && 2 == counts[2] && 2 == counts[3] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[3].point] + 1 == this->values[hand.cards[0].point] && JOKER_POINT != hand.cards[6].point) // && JOKER_POINT != hand.cards[8].point | 双王不是对子
				{
					hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
					return true;
				}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[8].point] + 4 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[9]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[9].point] + 9 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 11:
			candidates.clear();
			if (1 == counts[0] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[10].point] + 10 == this->values[hand.cards[0].point]) // && 1 == counts[1] && ... && 1 == counts[10]
			{
				hand.type = Type::SingleStraight; // 顺子
				return true;
			}
			else
				return false;
		case 12:
			switch (counts[0])
			{
			case 4:
				candidates.clear();
				switch (counts[1])
				{
				case 4:
					if (counts[2] >= 3) // && 1 == counts[3]
					{
						rotate(hand.cards.begin() + 7, hand.cards.begin() + 8, hand.cards.end());
						rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end() - 1);
						const Value value6 = this->values[hand.cards[6].point];
						if (value6 > this->values[hand.cards[3].point])
							rotate(value6 > this->values[hand.cards[0].point] ? hand.cards.begin() : hand.cards.begin() + 3, hand.cards.begin() + 6, hand.cards.begin() + 9);
						const Value value0 = this->values[hand.cards[0].point];
						if (value0 <= 12 && this->values[hand.cards[6].point] + 2 == value0)
						{
							const Value value9 = this->values[hand.cards[9].point];
							if (value9 < this->values[hand.cards[10].point])
								rotate(hand.cards.begin() + 9, hand.cards.begin() + 10, value9 < this->values[hand.cards[11].point] ? hand.cards.end() : hand.cards.end() - 1);
							hand.type = Type::TripleStraightWithSingles;
							return true;
						}
						else
							return false;
					}
					else
						return false;
				case 3:
					if (3 == counts[2])
					{
						rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end() - 2);
						const Value originalValue = this->values[hand.cards[0].point];
						if (originalValue < this->values[hand.cards[3].point])
							rotate(hand.cards.begin(), hand.cards.begin() + 3, originalValue < this->values[hand.cards[6].point] ? hand.cards.begin() + 9 : hand.cards.begin() + 6);
						const Value newValue = this->values[hand.cards[0].point];
						if (newValue <= 12 && this->values[hand.cards[6].point] + 2 == newValue)
						{
							const Value value9 = this->values[hand.cards[9].point];
							if (value9 < this->values[hand.cards[10].point])
								rotate(hand.cards.begin() + 9, hand.cards.begin() + 10, value9 < this->values[hand.cards[11].point] ? hand.cards.end() : hand.cards.end() - 1); // e.g., 9999888777KJ -> 9998887779KJ -> 999888777 + KJ9 | 9999888777J5 -> 9998887779J5 -> 999888777 + J95
							hand.type = Type::TripleStraightWithSingles;
							return true;
						}
						else
							return false;
					}
					else
						return false;
				default:
					return false;
				}
			case 3:
				if (3 == counts[1] && 3 == counts[2])
					switch (counts[3])
					{
					case 3:
						if (this->values[hand.cards[0].point] <= 12)
							if (this->values[hand.cards[9].point] + 3 == this->values[hand.cards[0].point])
							{
								vector<Candidate> potentialHands{};
								char buffers[2][3] = { 0 };
								this->convertPointToChars(hand.cards[0].point, buffers[0]);
								potentialHands.emplace_back(hand, (string)"解析为长度为 4 且点数为 " + buffers[0] + " 的飞机（``Type::TripleStraight``）");
								potentialHands.back().hand.type = Type::TripleStraight; // 飞机（不带翅膀）
								this->convertPointToChars(hand.cards[9].point, buffers[1]);
								potentialHands.emplace_back(hand, (string)"解析为长度为 3 且点数为 " + buffers[0] + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼的点数均为 " + buffers[1]);
								potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								this->convertPointToChars(hand.cards[3].point, buffers[1]);
								potentialHands.emplace_back(hand, (string)"解析为长度为 3 且点数为 " + buffers[1] + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼的点数均为 " + buffers[0]);
								rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, potentialHands.back().hand.cards.end());
								potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								if (this->lastHand && hand.player != this->lastHand.player)
									for (vector<Candidate>::iterator it = potentialHands.begin(); it != potentialHands.end();)
										if (this->coverLastHand(it->hand))
											++it;
										else
											it = potentialHands.erase(it);
								switch (potentialHands.size())
								{
								case 0:
									candidates.clear();
									return false;
								case 1:
									hand = move(potentialHands[0].hand);
									candidates.clear();
									return true;
								default:
									if (candidates.size() == 1)
									{
										const vector<Candidate>::iterator it = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
										if (it != potentialHands.end())
										{
											hand = move(it->hand);
											candidates.clear();
											return true;
										}
									}
									candidates = move(potentialHands);
									return false;
								}
							}
							else if (this->values[hand.cards[6].point] + 2 == this->values[hand.cards[0].point])
							{
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								candidates.clear();
								return true;
							}
						candidates.clear();
						if (this->values[hand.cards[9].point] + 2 == this->values[hand.cards[3].point])
						{
							rotate(hand.cards.begin(), hand.cards.begin() + 3, hand.cards.end()); // e.g., 222999888777 -> 999888777 + 222
							hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
							return true;
						}
						else
							return false;
					case 2: // && 1 == counts[4]
						candidates.clear();
						if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 2 == this->values[hand.cards[0].point])
						{
							if (this->values[hand.cards[9].point] < this->values[hand.cards[11].point])
								rotate(hand.cards.begin() + 9, hand.cards.begin() + 11, hand.cards.end()); // e.g., 999888777335 -> 999888777 + 533
							hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
							return true;
						}
						else
							return false;
					case 1: // && 1 == counts[4] && 1 == counts[5]
						candidates.clear();
						if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 2 == this->values[hand.cards[0].point])
						{
							hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
							return true;
						}
						else
							return false;
					default:
						candidates.clear();
						return false;
					}
				else
				{
					candidates.clear();
					return false;
				}
			case 2:
				candidates.clear();
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[10].point] + 5 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[11]
				candidates.clear();
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[11].point] + 11 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				candidates.clear();
				return false;
			}
		case 14:
			candidates.clear();
			if (2 == counts[0] && 2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[12].point] + 6 == this->values[hand.cards[0].point])
			{
				hand.type = Type::PairStraight; // 连对
				return true;
			}
			else
				return false;
		case 15:
			candidates.clear();
			switch (counts[0])
			{
			case 4:
				if (3 == counts[1] && 3 == counts[2] && 3 == counts[4] && 2 == counts[5] && this->values[hand.cards[4].point] <= 12 && this->values[hand.cards[10].point] + 2 == this->values[hand.cards[4].point] && JOKER_POINT != hand.cards[13].point) // 双王不是对子
				{
					rotate(hand.cards.begin(), hand.cards.begin() + 4, hand.cards.begin() + 13);
					hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
					return true;
				}
				else
					return false;
			case 3:
				if (3 == counts[1] && 3 == counts[2])
					switch (counts[3])
					{
					case 3:
						if (3 == counts[4] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
						{
							hand.type = Type::TripleStraight; // 飞机（不带翅膀）
							return true;
						}
						else
							return false;
					case 2:
						if (2 == counts[4] && 2 == counts[5] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 2 == this->values[hand.cards[0].point] && JOKER_POINT != hand.cards[9].point) // && JOKER_POINT != hand.cards[11].point && JOKER_POINT != hand.cards[13].point | 双王不是对子
						{
							hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
							return true;
						}
						else
							return false;
					default:
						return false;
					}
				else
					return false;
			default:
				return false;
			}
		case 16:
			switch (counts[0])
			{
			case 4:
				switch (counts[1])
				{
				case 4:
					candidates.clear();
					switch (counts[2])
					{
					case 4:
						switch (counts[3])
						{
						case 4:
						{
							rotate(hand.cards.begin() + 11, hand.cards.begin() + 12, hand.cards.end() - 1);
							rotate(hand.cards.begin() + 7, hand.cards.begin() + 8, hand.cards.end() - 2);
							rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end() - 3);
							const Value value0 = this->values[hand.cards[0].point];
							if (value0 <= 12 && this->values[hand.cards[9].point] + 3 == value0)
							{
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								return true;
							}
							else
								return false;
						}
						case 3: // && 1 == counts[4]
						{
							rotate(hand.cards.begin() + 11, hand.cards.begin() + 12, hand.cards.end() - 1);
							rotate(hand.cards.begin() + 7, hand.cards.begin() + 8, hand.cards.end() - 2);
							rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end() - 3);
							const Value value0 = this->values[hand.cards[0].point];
							if (value0 <= 12 && this->values[hand.cards[9].point] + 3 == value0)
							{
								const Value value15 = this->values[hand.cards[15].point];
								if (value15 > this->values[hand.cards[14].point])
									rotate(hand.cards.begin() + (value15 > this->values[hand.cards[13].point] ? (value15 > value0 ? 12 : 13) : 14), hand.cards.begin() + 15, hand.cards.end());
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								return true;
							}
							else
								return false;
						}
						default:
							return false;
						}
					case 3:
						if (counts[3] >= 3)
						{
							rotate(hand.cards.begin() + 7, hand.cards.begin() + 8, hand.cards.end());
							rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end() - 1);
							const Value value0 = this->values[hand.cards[0].point];
							if (value0 <= 12 && this->values[hand.cards[9].point] + 3 == value0)
							{
								if (this->values[hand.cards[12].point] < value0)
									rotate(hand.cards.begin() + 12, hand.cards.begin() + 14, hand.cards.end());
								const Value value13 = this->values[hand.cards[13].point];
								if (value13 < this->values[hand.cards[14].point])
									rotate(hand.cards.begin() + 13, hand.cards.begin() + 14, value13 < this->values[hand.cards[15].point] ? hand.cards.end() : hand.cards.end() - 1);
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								return true;
							}
							else
								return false;
						}
						else
							return false;
					default:
						return false;
					}
				case 3:
					if (3 == counts[2] && 3 == counts[3])
						if (3 == counts[4])
						{
							rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end());
							vector<Card>::iterator cardIt = hand.cards.begin() + 3;
							const vector<Card>::iterator indexToLastBodyPoint = hand.cards.begin() + 12;
							const Value value = this->values[hand.cards[0].point];
							while (cardIt <= indexToLastBodyPoint && this->values[cardIt->point] > value)
								cardIt += 3;
							rotate(hand.cards.begin(), hand.cards.begin() + 3, cardIt);
							if (this->values[hand.cards[0].point] <= 12)
								if (this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
								{
									vector<Candidate> potentialHands{};
									char buffers[2][3] = { 0 };
									this->convertPointToChars(hand.cards[0].point, buffers[0]);
									this->convertPointToChars(hand.cards[12].point, buffers[1]);
									potentialHands.emplace_back(hand, (string)"解析为长度为 4 且点数为 " + buffers[0] + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + buffers[1] + " 的牌");
									if (this->values[hand.cards[12].point] < this->values[hand.cards[15].point])
										rotate(potentialHands.back().hand.cards.begin() + 12, potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.end()); // e.g., 9999888777666555 -> 9998887776665559 -> 999888777666 + 9555 | else e.g., 5555999888777666 -> 9998887776665555 -> 999888777666 + 5555
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									this->convertPointToChars(hand.cards[3].point, buffers[1]);
									potentialHands.emplace_back(hand, (string)"解析为长度为 4 且点数为 " + buffers[1] + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + buffers[0] + " 的牌");
									rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, potentialHands.back().hand.cards.end() - 1); // e.g., 8888999777666555 -> 9998887776665558 -> 888777666555 + 9998
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									if (this->lastHand && hand.player != this->lastHand.player)
										for (vector<Candidate>::iterator candidateIt = potentialHands.begin(); candidateIt != potentialHands.end();)
											if (this->coverLastHand(candidateIt->hand))
												++candidateIt;
											else
												candidateIt = potentialHands.erase(candidateIt);
									switch (potentialHands.size())
									{
									case 0:
										candidates.clear();
										return false;
									case 1:
										hand = move(potentialHands[0].hand);
										candidates.clear();
										return true;
									default:
										if (candidates.size() == 1)
										{
											const vector<Candidate>::iterator candidateIt = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
											if (candidateIt != potentialHands.end())
											{
												hand = candidateIt->hand;
												candidates.clear();
												return true;
											}
										}
										candidates = move(potentialHands);
										return false;
									}
								}
								else if (this->values[hand.cards[9].point] + 3 == this->values[hand.cards[0].point])
								{
									rotate(hand.cards.begin() + 12, hand.cards.begin() + 15, hand.cards.end()); // if (this->values[hand.cards[12].point] < this->values[hand.cards[15].point]) | e.g., 8888999777666444 -> 9998887776664448 -> 999888777666 + 8444
									hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									candidates.clear();
									return true;
								}
							candidates.clear();
							if (this->values[hand.cards[12].point] + 3 == this->values[hand.cards[3].point])
							{
								rotate(hand.cards.begin(), hand.cards.begin() + 3, hand.cards.end() - 1); // e.g., 2222999888777666 -> 2229998887776662 -> 999888777666 + 2222
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								return true;
							}
							else
								return false;
						}
						else
						{
							candidates.clear();
							rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.begin() + 13);
							vector<Card>::iterator it = hand.cards.begin() + 3;
							const vector<Card>::iterator indexToLastBodyPoint = hand.cards.begin() + 9;
							const Value originalValue = this->values[hand.cards[0].point];
							while (it <= indexToLastBodyPoint && this->values[it->point] > originalValue)
								it += 3;
							rotate(hand.cards.begin(), hand.cards.begin() + 3, it);
							const Value newValue = this->values[hand.cards[0].point];
							if (newValue <= 12 && this->values[hand.cards[9].point] + 3 == newValue)
							{
								sort(hand.cards.begin() + 13, hand.cards.end(), [this](const Card a, const Card b) { const Value valueA = this->values[a.point], valueB = this->values[b.point]; return valueA > valueB || (valueA == valueB && a.suit > b.suit); });
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								return true;
							}
							else
								return false;
						}
					else
					{
						candidates.clear();
						return false;
					}
				default:
					candidates.clear();
					return false;
				}
			case 3:
				if (3 == counts[1] && 3 == counts[2] && 3 == counts[3])
					switch (counts[4])
					{
					case 3: // && 1 == counts[4]
						if (this->values[hand.cards[0].point] <= 12)
							if (this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
							{
								vector<Candidate> potentialHands{};
								char buffers[2][3] = { 0 };
								this->convertPointToChars(hand.cards[0].point, buffers[0]);
								this->convertPointToChars(hand.cards[12].point, buffers[1]);
								potentialHands.emplace_back(hand, (string)"解析为长度为 4 且点数为 " + buffers[0] + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + buffers[1] + " 的牌");
								if (this->values[hand.cards[12].point] < this->values[hand.cards[15].point])
									rotate(potentialHands.back().hand.cards.begin() + 12, potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.end()); // e.g., 999888777666555K -> 999888777666 + K555
								potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								this->convertPointToChars(hand.cards[3].point, buffers[1]);
								potentialHands.emplace_back(hand, (string)"解析为长度为 4 且点数为 " + buffers[1] + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + buffers[0] + " 的牌");
								rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, hand.cards[0].point < hand.cards[15].point ? potentialHands.back().hand.cards.end() : potentialHands.back().hand.cards.end() - 1); // e.g., 999888777666555K -> 888777666555 + K999 | 9998887776665553 -> 888777666555 + 9993
								potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								if (this->lastHand && hand.player != this->lastHand.player)
									for (vector<Candidate>::iterator it = potentialHands.begin(); it != potentialHands.end();)
										if (this->coverLastHand(it->hand))
											++it;
										else
											it = potentialHands.erase(it);
								switch (potentialHands.size())
								{
								case 0:
									candidates.clear();
									return false;
								case 1:
									hand = move(potentialHands[0].hand);
									candidates.clear();
									return true;
								default:
									if (candidates.size() == 1)
									{
										const vector<Candidate>::iterator it = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
										if (it != potentialHands.end())
										{
											hand = move(it->hand);
											candidates.clear();
											return true;
										}
									}
									candidates = move(potentialHands);
									return false;
								}
							}
							else if (this->values[hand.cards[9].point] + 3 == this->values[hand.cards[0].point])
							{
								if (this->values[hand.cards[12].point] < this->values[hand.cards[15].point])
									rotate(hand.cards.begin() + 12, hand.cards.begin() + 15, hand.cards.end()); // e.g., AAAKKKQQQJJJ3335 -> AAAKKKQQQJJJ + 5333
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								candidates.clear();
								return true;
							}
						candidates.clear();
						if (this->values[hand.cards[12].point] + 3 == this->values[hand.cards[3].point])
						{
							rotate(hand.cards.begin(), hand.cards.begin() + 3, this->values[hand.cards[0].point] < this->values[hand.cards[15].point] ? hand.cards.end() : hand.cards.end() - 1); // e.g., KKK9998887776662 -> 999888777666 + 2KKK | 222999888777666K -> 999888777666 + 222K
							hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
							return true;
						}
						else
							return false;
					case 2:
					{
						candidates.clear();
						const Value value = this->values[hand.cards[0].point];
						if (value <= 12 && this->values[hand.cards[9].point] + 3 == value)
						{
							if (1 == counts[5] && this->values[hand.cards[12].point] < this->values[hand.cards[14].point]) // && 1 == counts[6]
								rotate(hand.cards.begin() + 12, hand.cards.begin() + 14, this->values[hand.cards[12].point] < this->values[hand.cards[15].point] ? hand.cards.end() : hand.cards.end() - 1); // e.g., AAAKKKQQQJJJ3375 -> AAAKKKQQQJJJ + 7533 | AAAKKKQQQJJJ5573 -> AAAKKKQQQJJJ + 7553
							hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
							return true;
						}
						else
							return false;
					}
					case 1: // && 1 == counts[5] && 1 == counts[6] && 1 == counts[7]
					{
						candidates.clear();
						const Value value = this->values[hand.cards[0].point];
						if (value <= 12 && this->values[hand.cards[9].point] + 3 == value)
						{
							hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
							return true;
						}
						else
							return false;
					}
					default:
						candidates.clear();
						return false;
					}
				else
				{
					candidates.clear();
					return false;
				}
			case 2:
				candidates.clear();
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && 2 == counts[7] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[14].point] + 7 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			default:
				candidates.clear();
				return false;
			}
		case 18:
			candidates.clear();
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 3 == counts[2] && 3 == counts[3] && 3 == counts[4] && 3 == counts[5] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[15].point] + 5 == this->values[hand.cards[0].point])
				{
					hand.type = Type::TripleStraight; // 飞机（不带翅膀）
					return true;
				}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && 2 == counts[7] && 2 == counts[8] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[16].point] + 8 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 20:
			switch (counts[0])
			{
			case 4:
				switch (counts[1])
				{
				case 4:
					switch (counts[2])
					{
					case 4:
						candidates.clear();
						switch (counts[3])
						{
						case 4:
							switch (counts[4])
							{
							case 4:
							{
								const Value value = this->values[hand.cards[0].point];
								if (value <= 12 && this->values[hand.cards[16].point] + 4 == value)
								{
									rotate(hand.cards.begin() + 15, hand.cards.begin() + 16, hand.cards.end() - 1);
									rotate(hand.cards.begin() + 11, hand.cards.begin() + 12, hand.cards.end() - 2);
									rotate(hand.cards.begin() + 7, hand.cards.begin() + 8, hand.cards.end() - 3);
									rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end() - 4);
									hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									return true;
								}
								else
									return false;
							}
							case 3:
							{
								rotate(hand.cards.begin() + 15, hand.cards.begin() + 16, hand.cards.end() - 1);
								rotate(hand.cards.begin() + 11, hand.cards.begin() + 12, hand.cards.end() - 2);
								rotate(hand.cards.begin() + 7, hand.cards.begin() + 8, hand.cards.end() - 3);
								rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end() - 4);
								vector<Card>::iterator it = hand.cards.begin() + 12;
								const vector<Card>::iterator indexToSecondBodyPoint = hand.cards.begin() + 3;
								const Value value12 = this->values[hand.cards[12].point];
								while (it >= indexToSecondBodyPoint && this->values[(it - 3)->point] < value12)
									it -= 3;
								rotate(it, hand.cards.begin() + 12, hand.cards.begin() + 15);
								const Value value0 = this->values[hand.cards[0].point];
								if (value0 <= 12 && this->values[hand.cards[12].point] + 4 == value0)
								{
									const vector<Card>::iterator indexToSecondSidePoint = hand.cards.begin() + 16;
									const Value value19 = this->values[hand.cards[19].point];
									for (it = hand.cards.begin() + 19; it >= indexToSecondSidePoint && this->values[(it - 1)->point] < value19; --it);
									rotate(it, hand.cards.begin() + 19, hand.cards.end());
									hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									return true;
								}
								else
									return false;
							}
							default:
								return false;
							}
						case 3:
							if (3 == counts[4])
							{
								rotate(hand.cards.begin() + 11, hand.cards.begin() + 12, hand.cards.end());
								rotate(hand.cards.begin() + 7, hand.cards.begin() + 8, hand.cards.end() - 1);
								rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end() - 2);
								vector<Card>::iterator it = hand.cards.begin() + 9;
								const vector<Card>::iterator indexToSecondBodyPoint = hand.cards.begin() + 3;
								const Value value9 = this->values[hand.cards[9].point], value12 = this->values[hand.cards[12].point];
								while (it >= indexToSecondBodyPoint && this->values[(it - 3)->point] < value9)
									it -= 3;
								rotate(it, hand.cards.begin() + 9, hand.cards.begin() + 12);
								it += 3;
								vector<Card>::iterator secondaryIt = hand.cards.begin() + 12;
								while (secondaryIt > it && this->values[(secondaryIt - 3)->point] < value12)
									secondaryIt -= 3;
								rotate(secondaryIt, hand.cards.begin() + 12, hand.cards.begin() + 15);
								const Value value0 = this->values[hand.cards[0].point];
								if (value0 <= 12 && this->values[hand.cards[12].point] + 4 == value0)
								{
									const Value value15 = this->values[hand.cards[15].point], value16 = this->values[hand.cards[16].point];
									for (it = hand.cards.begin() + 17; it != hand.cards.end() && this->values[it->point] > value16; ++it);
									rotate(hand.cards.begin() + 16, hand.cards.begin() + 17, it);
									--it;
									for (secondaryIt = hand.cards.begin() + 16; secondaryIt < it && this->values[secondaryIt->point] > value15; ++secondaryIt);
									rotate(hand.cards.begin() + 15, hand.cards.begin() + 16, secondaryIt);
									hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									return true;
								}
								else
									return false;
							}
							else
								return false;
						default:
							return false;
						}
					case 3:
						if (3 == counts[3] && 3 == counts[4])
							if (3 == counts[5])
							{

							}
							else
							{
								candidates.clear();
								rotate(hand.cards.begin() + 7, hand.cards.begin() + 8, hand.cards.end() - 3);
								rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end() - 4);
								vector<Card>::iterator it = hand.cards.begin() + 6;
								const vector<Card>::iterator indexToLastBodyPoint = hand.cards.begin() + 12;
								const Value originalValue = this->values[hand.cards[0].point], value3 = this->values[hand.cards[3].point];
								while (it <= indexToLastBodyPoint && this->values[it->point] > value3)
									it += 3;
								rotate(hand.cards.begin() + 3, hand.cards.begin() + 6, it);
								vector<Card>::iterator secondaryIt = hand.cards.begin() + 3;
								it -= 3;
								while (secondaryIt < it && this->values[secondaryIt->point] > originalValue)
									secondaryIt += 3;
								rotate(hand.cards.begin(), hand.cards.begin() + 3, secondaryIt);
								const Value newValue = this->values[hand.cards[0].point];
								if (newValue <= 12 && this->values[hand.cards[12].point] + 4 == newValue)
								{
									sort(hand.cards.begin() + 15, hand.cards.end(), [this](const Card a, const Card b) { const Value valueA = this->values[a.point], valueB = this->values[b.point]; return valueA > valueB || (valueA == valueB && a.suit > b.suit); });
									hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									return true;
								}
								else
									return false;
							}
						else
						{
							candidates.clear();
							return false;
						}
					default:
						candidates.clear();
						return false;
					}
				case 3:
					if (3 == counts[2] && 3 == counts[3] && 3 == counts[4])
						if (3 == counts[5])
						{
							const Value originalValue = this->values[hand.cards[0].point];
							const bool isValue19Larger = originalValue < this->values[hand.cards[19].point];
							rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, isValue19Larger ? hand.cards.end() : hand.cards.end() - 1);
							vector<Card>::iterator it = hand.cards.begin() + 3;
							const vector<Card>::iterator indexToLastBodyPoint = hand.cards.begin() + 15;
							while (it <= indexToLastBodyPoint && this->values[it->point] > originalValue)
								it += 3;
							rotate(hand.cards.begin(), hand.cards.begin() + 3, it);
							const Value newValue = this->values[hand.cards[0].point];
							if (newValue <= 12)
								if (this->values[hand.cards[15].point] + 5 == newValue)
								{
									const Value value15 = this->values[hand.cards[15].point];
									vector<Candidate> potentialHands{};
									char buffers[2][3] = { 0 };
									this->convertPointToChars(hand.cards[0].point, buffers[0]);
									this->convertPointToChars(hand.cards[15].point, buffers[1]);
									potentialHands.emplace_back(hand, (string)"解析为长度为 5 且点数为 " + buffers[0] + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + buffers[1] + " 的牌");
									if (value15 < this->values[hand.cards[18].point])
										rotate(potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.begin() + 18, value15 < this->values[hand.cards[19].point] ? potentialHands.back().hand.cards.end() : potentialHands.back().hand.cards.end() - 1);
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									this->convertPointToChars(hand.cards[3].point, buffers[1]);
									potentialHands.emplace_back(hand, (string)"解析为长度为 5 且点数为 " + buffers[1] + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + buffers[0] + " 的牌");
									rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, isValue19Larger ? potentialHands.back().hand.cards.end() - 1 : potentialHands.back().hand.cards.end() - 2);
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									if (this->lastHand && hand.player != this->lastHand.player)
										for (vector<Candidate>::iterator candidateIt = potentialHands.begin(); candidateIt != potentialHands.end();)
											if (this->coverLastHand(candidateIt->hand))
												++candidateIt;
											else
												candidateIt = potentialHands.erase(candidateIt);
									switch (potentialHands.size())
									{
									case 0:
										candidates.clear();
										return false;
									case 1:
										hand = move(potentialHands[0].hand);
										candidates.clear();
										return true;
									default:
										if (candidates.size() == 1)
										{
											const vector<Candidate>::iterator candidateIt = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
											if (candidateIt != potentialHands.end())
											{
												hand = candidateIt->hand;
												candidates.clear();
												return true;
											}
										}
										candidates = move(potentialHands);
										return false;
									}
								}
								else if (this->values[hand.cards[12].point] + 4 == newValue)
								{
									const Value value15 = this->values[hand.cards[15].point];
									if (value15 < this->values[hand.cards[18].point])
										rotate(hand.cards.begin() + 15, hand.cards.begin() + 18, value15 < this->values[hand.cards[19].point] ? hand.cards.end() : hand.cards.end() - 1); // e.g., QQQQAAAKKKJJJTTT8883 -> QQQAAAKKKJJJTTT888Q3 -> AAAKKKQQQJJJTTT999Q3 -> AAAKKKQQQJJJTTT + Q9993
									hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									candidates.clear();
									return true;
								}
							candidates.clear();
							if (this->values[hand.cards[15].point] + 4 == this->values[hand.cards[3].point])
							{
								for (it = hand.cards.begin() + 18; it != hand.cards.end() && this->values[it->point] > newValue; ++it);
								rotate(hand.cards.begin(), hand.cards.begin() + 3, it); // e.g., 6666JJJ9998887775553 -> 666JJJ99988877755563 -> JJJ99988877766655563 -> 999888777666555 + JJJ63
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								return true;
							}
							else
								return false;
						}
						else
						{
							/* Fetch 3333 + 4(2 + 2) + 2 + 2 */
							const Value originalValue = this->values[hand.cards[0].point], value4 = this->values[hand.cards[4].point];
							vector<Candidate> potentialHands{};
							if (2 == counts[5] && 2 == counts[6] && JOKER_POINT != hand.cards[16].point && value4 <= 12 && this->values[hand.cards[13].point] + 3 == value4)
							{
								char buffers[2][3] = { 0 };
								this->convertPointToChars(hand.cards[4].point, buffers[0]);
								this->convertPointToChars(hand.cards[0].point, buffers[1]);
								potentialHands.emplace_back(hand, (string)"解析为长度为 4 且点数为 " + buffers[0] + " 的飞机带大翼（``Type::TripleStraightWithPairs``），其中大翼包含两个点数为 " + buffers[1] + " 的对子");
								rotate(potentialHands.back().hand.cards.begin(), potentialHands[0].hand.cards.begin() + 4, originalValue < this->values[potentialHands.back().hand.cards[16].point] ? (originalValue < this->values[potentialHands.back().hand.cards[18].point] ? potentialHands.back().hand.cards.end() : potentialHands.back().hand.cards.end() - 2) : potentialHands.back().hand.cards.end() - 4);
								potentialHands.back().hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
							}
							
							/* Fetch 43333... -> 3 * 5 + 1 * 5 */
							rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end() - 4);
							vector<Card>::iterator it = hand.cards.begin() + 3;
							const vector<Card>::iterator indexToLastBodyPoint = hand.cards.begin() + 12;
							while (it <= indexToLastBodyPoint && this->values[it->point] > originalValue)
								it += 3;
							rotate(hand.cards.begin(), hand.cards.begin() + 3, it);
							const Value newValue = this->values[hand.cards[0].point];
							if (newValue <= 12 && this->values[hand.cards[12].point] + 4 == newValue)
							{
								sort(hand.cards.begin() + 15, hand.cards.end(), [this](const Card a, const Card b) { const Value valueA = this->values[a.point], valueB = this->values[b.point]; return valueA > valueB || (valueA == valueB && a.suit > b.suit); });
								if (potentialHands.empty())
								{
									hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									candidates.clear();
									return true;
								}
								else
								{
									char buffer[3] = { 0 };
									this->convertPointToChars(hand.cards[0].point, buffer);
									potentialHands.emplace_back(hand, (string)"解析为长度为 5 且点数为 " + buffer + " 的飞机带小翼（``Type::TripleStraightWithSingles``）");
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									if (this->lastHand && hand.player != this->lastHand.player)
										for (vector<Candidate>::iterator candidateIt = potentialHands.begin(); candidateIt != potentialHands.end();)
											if (this->coverLastHand(candidateIt->hand))
												++candidateIt;
											else
												candidateIt = potentialHands.erase(candidateIt);
									switch (potentialHands.size())
									{
									case 0:
										candidates.clear();
										return false;
									case 1:
										hand = move(potentialHands[0].hand);
										candidates.clear();
										return true;
									default:
										if (candidates.size() == 1)
										{
											const vector<Candidate>::iterator candidateIt = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
											if (candidateIt != potentialHands.end())
											{
												hand = candidateIt->hand;
												candidates.clear();
												return true;
											}
										}
										candidates = move(potentialHands);
										return false;
									}
								}
							}
							else if (potentialHands.empty())
							{
								candidates.clear();
								return false;
							}
							else
							{
								hand = move(potentialHands[0].hand);
								candidates.clear();
								return true;
							}
						}
					else
					{
						candidates.clear();
						return false;
					}
				default:
					candidates.clear();
					return false;
				}
			case 3:
				if (3 == counts[1] && 3 == counts[2] && 3 == counts[3])
					switch (counts[4])
					{
					case 3:
						switch (counts[5])
						{
						case 3:
							if (this->values[hand.cards[0].point] <= 12)
								if (this->values[hand.cards[15].point] + 5 == this->values[hand.cards[0].point])
								{
									const Value value0 = this->values[hand.cards[0].point], value15 = this->values[hand.cards[15].point];
									vector<Candidate> potentialHands{};
									char buffers[2][3] = { 0 };
									this->convertPointToChars(hand.cards[0].point, buffers[0]);
									this->convertPointToChars(hand.cards[15].point, buffers[1]);
									potentialHands.emplace_back(hand, (string)"解析为长度为 5 且点数为 " + buffers[0] + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + buffers[1] + " 的牌");
									if (value15 < this->values[hand.cards[18].point])
										rotate(potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.begin() + 18, value15 < this->values[hand.cards[19].point] ? potentialHands.back().hand.cards.end() : potentialHands.back().hand.cards.end() - 1); // e.g., 999888777666555444KK -> 999888777666555 + KK444 | 999888777666555444K3 -> 999888777666555 + K4443
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									this->convertPointToChars(hand.cards[3].point, buffers[1]);
									potentialHands.emplace_back(hand, (string)"解析为长度为 5 且点数为 " + buffers[1] + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + buffers[0] + " 的牌");
									rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, value0 < this->values[hand.cards[18].point] ? (value0 < this->values[hand.cards[19].point] ? potentialHands.back().hand.cards.end() : potentialHands.back().hand.cards.end() - 1) : potentialHands.back().hand.cards.end() - 2); // e.g., 999888777666555444KK -> 888777666555444 + KK999 | 999888777666555444K3 -> 888777666555444 + K9993 | AAAKKKQQQJJJTTT99973 -> KKKQQQJJJTTT999 + AAA73
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									if (this->lastHand && hand.player != this->lastHand.player)
										for (vector<Candidate>::iterator candidateIt = potentialHands.begin(); candidateIt != potentialHands.end();)
											if (this->coverLastHand(candidateIt->hand))
												++candidateIt;
											else
												candidateIt = potentialHands.erase(candidateIt);
									switch (potentialHands.size())
									{
									case 0:
										candidates.clear();
										return false;
									case 1:
										hand = move(potentialHands[0].hand);
										candidates.clear();
										return true;
									default:
										if (candidates.size() == 1)
										{
											const vector<Candidate>::iterator candidateIt = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
											if (candidateIt != potentialHands.end())
											{
												hand = candidateIt->hand;
												candidates.clear();
												return true;
											}
										}
										candidates = move(potentialHands);
										return false;
									}
								}
								else if (this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
								{
									const Value value15 = this->values[hand.cards[15].point];
									if (value15 < this->values[hand.cards[18].point])
										rotate(hand.cards.begin() + 15, hand.cards.begin() + 18, value15 < this->values[hand.cards[19].point] ? hand.cards.end() : hand.cards.end() - 1); // e.g., KKK...99933355 -> KKK...999 + 55333 | KKK...99944453 -> KKK...999 + 54443
									hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									candidates.clear();
									return true;
								}
							candidates.clear();
							if (this->values[hand.cards[15].point] + 4 == this->values[hand.cards[3].point])
							{
								const Value value0 = this->values[hand.cards[0].point];
								rotate(hand.cards.begin(), hand.cards.begin() + 3, value0 < this->values[hand.cards[18].point] ? (value0 < this->values[hand.cards[19].point] ? hand.cards.end() : hand.cards.end() - 1) : hand.cards.end() - 2); // e.g., JJJ9998887776665552K -> 999888777666555 + 2KJJJ | KKK9998887776665552J -> 999888777666555 + 2KKKJ | 222999888777666555KJ -> 999888777666555 + 222KJ
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								return true;
							}
							else
								return false;
						case 2:
							candidates.clear();
							if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
							{
								sort(hand.cards.begin() + 15, hand.cards.end(), [this](const Card a, const Card b) { const Value valueA = this->values[a.point], valueB = this->values[b.point]; return valueA > valueB || (valueA == valueB && a.suit > b.suit); });
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								return true;
							}
							else
								return false;
						case 1: // && 1 == counts[6] && 1 == counts[7] && 1 == counts[8] && 1 == counts[9]
							hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
							candidates.clear();
							return true;
						default:
							candidates.clear();
							return false;
						}
					case 2:
						if (2 == counts[5] && 2 == counts[6] && 2 == counts[7] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[9].point] + 3 == this->values[hand.cards[0].point] && JOKER_POINT != hand.cards[12].point) // && JOKER_POINT != hand.cards[14].point && JOKER_POINT != hand.cards[16].point && JOKER_POINT != hand.cards[18].point | 双王不是对子
						{
							hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
							return true;
						}
						else
							return false;
					default:
						return false;
					}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && 2 == counts[7] && 2 == counts[8] && 2 == counts[9] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[18].point] + 9 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		default:
			candidates.clear();
			return false;
		}
	}

public:
	LandlordsX() : Landlords()
	{
		this->pokerType = "斗地主拓展版";
	}
};

class Landlords4P : public PokerGame /* Previous: LandlordsX | Next: BigTwo */
{
private:
	const bool assignDealer() override final
	{
		if (Status::Dealt == this->status && this->records.empty())
		{
			this->records.push_back(vector<Hand>{});
			uniform_int_distribution<size_t> distribution(0, this->players.size() - 1);
			this->currentPlayer = (Player)(distribution(this->seed));
			this->dealer = INVALID_PLAYER;
			this->lastHand = Hand{};
			this->amounts.clear();
			return true;
		}
		else
			return false;
	}
	const bool checkStarting(const vector<Card>& cards) const override final
	{
		return !cards.empty();
	}
	const bool processHand(Hand& hand, vector<Candidate>& candidates) const override final
	{
		hand.type = Type::Invalid;
		candidates.clear();
		Count littleJokerCount = 0, bigJokerCount = 0;
		vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (JOKER_POINT == card.point)
			{
				switch (card.suit)
				{
				case Suit::Black:
					if (++littleJokerCount > 2)
						return false;
				case Suit::Red:
					if (++bigJokerCount > 2)
						return false;
				default:
					return false;
				}
			}
			else if (this->values[card.point])
				++counts[card.point];
			else
				return false;
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && valueA > valueB) || (countA == countB && valueA == valueB && a.suit > b.suit); });
		sort(counts.begin(), counts.end(), [](const Count a, const Count b) { return a > b; });
		if (counts[0] > 8)
			return false;
		switch (hand.cards.size())
		{
		case 0:
			hand.type = Type::Empty; // 要不起
			return true;
		case 1:
			hand.type = Type::Single; // 单牌
			return true;
		case 2:
			if (2 == counts[0] && (JOKER_POINT != hand.cards[0].point || hand.cards[0].suit == hand.cards[1].suit))
			{
				hand.type = Type::Pair; // 对子
				return true;
			}
			else
				return false;
		case 3:
			if (3 == counts[0] && JOKER_POINT != hand.cards[0].point)
			{
				hand.type = Type::Triple; // 三条
				return true;
			}
			else
				return false;
		case 4:
			if (4 == counts[0])
			{
				hand.type = JOKER_POINT == hand.cards[0].point ? Type::QuadrupleJokers : Type::Quadruple; // 天王炸弹 | 四条
				return true;
			}
			else
				return false;
		case 5:
			switch (counts[0])
			{
			case 5:
				hand.type = Type::Quintuple; // 五张炸弹
				return true;
			case 3:
				if (2 == counts[1] && JOKER_POINT != hand.cards[0].point && (JOKER_POINT != hand.cards[3].point || hand.cards[3].suit == hand.cards[4].suit))
				{
					hand.type = Type::TripleWithPair; // 三带一对
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && 1 == counts[2] && 1 == counts[3] && 1 == counts[4]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[4].point] + 4 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			}
		case 6:
			switch (counts[0])
			{
			case 6:
				hand.type = Type::Sextuple; // 六张炸弹
				return true;
			case 3:
				if (3 == counts[1] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[3].point] + 1 == this->values[hand.cards[0].point])
				{
					hand.type = Type::TripleStraight; // 飞机（不带翅膀）
					return true;
				}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[4].point] + 2 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[5]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[5].point] + 5 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 7:
			switch (counts[0])
			{
			case 7:
				hand.type = Type::Septuple; // 七张炸弹
				return true;
			case 1: // && 1 == counts[1] && ... && 1 == counts[6]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 6 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 8:
			switch (counts[0])
			{
			case 8:
				hand.type = Type::Octuple; // 八张炸弹
				return true;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 3 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[7]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[7].point] + 7 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 9:
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 3 == counts[2] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 2 == this->values[hand.cards[0].point])
				{
					hand.type = Type::TripleStraight; // 飞机（不带翅膀）
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[8]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[8].point] + 8 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 10:
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 2 == counts[2] && 2 == counts[3] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[3].point] + 1 == this->values[hand.cards[0].point] && (JOKER_POINT != hand.cards[6].point || hand.cards[6].suit == hand.cards[7].suit))
				{
					hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
					return true;
				}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[8].point] + 4 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[9]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[9].point] + 9 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 11:
			if (1 == counts[0] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[10].point] + 10 == this->values[hand.cards[0].point]) // && 1 == counts[1] && ... && 1 == counts[10]
			{
				hand.type = Type::SingleStraight; // 顺子
				return true;
			}
			else
				return false;
		case 12:
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 3 == counts[2] && 3 == counts[3] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[9].point] + 3 == this->values[hand.cards[0].point])
				{
					hand.type = Type::TripleStraight; // 飞机（不带翅膀）
					return true;
				}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[10].point] + 5 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && ... && 1 == counts[11]
				if (this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[11].point] + 11 == this->values[hand.cards[0].point])
				{
					hand.type = Type::SingleStraight; // 顺子
					return true;
				}
				else
					return false;
			}
		case 14:
			if (2 == counts[0] && 2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[12].point] + 6 == this->values[hand.cards[0].point])
			{
				hand.type = Type::PairStraight; // 连对
				return true;
			}
			else
				return false;
		case 15:
			if (3 == counts[0] && 3 == counts[1] && 3 == counts[2])
				switch (counts[3])
				{
				case 3:
					if (3 == counts[4] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
					{
						hand.type = Type::TripleStraight; // 飞机（不带翅膀）
						return true;
					}
					else
						return false;
				case 2:
					if (2 == counts[4] && 2 == counts[5] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[6].point] + 2 == this->values[hand.cards[0].point] && (JOKER_POINT != hand.cards[9].point || hand.cards[9].suit == hand.cards[10].suit))
					{
						hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
						return true;
					}
					else
						return false;
				default:
					return false;
				}
			else
				return false;
		case 16:
			if (2 == counts[0] && 2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && 2 == counts[7] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[14].point] + 7 == this->values[hand.cards[0].point])
			{
				hand.type = Type::PairStraight; // 连对
				return true;
			}
			else
				return false;
		case 18:
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 3 == counts[2] && 3 == counts[3] && 3 == counts[4] && 3 == counts[5] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[15].point] + 5 == this->values[hand.cards[0].point])
				{
					hand.type = Type::TripleStraight; // 飞机（不带翅膀）
					return true;
				}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && 2 == counts[7] && 2 == counts[8] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[16].point] + 8 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 20:
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 3 == counts[2] && 3 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && 2 == counts[7] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[9].point] + 3 == this->values[hand.cards[0].point] && (JOKER_POINT != hand.cards[12].point || hand.cards[12].suit == hand.cards[13].suit))
				{
					hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
					return true;
				}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && 2 == counts[7] && 2 == counts[8] && 2 == counts[9] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[18].point] + 9 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 22:
			if (2 == counts[0] && 2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && 2 == counts[7] && 2 == counts[8] && 2 == counts[9] && 2 == counts[10] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[20].point] + 10 == this->values[hand.cards[0].point])
			{
				hand.type = Type::PairStraight; // 连对
				return true;
			}
			else
				return false;
		case 24:
			switch (counts[0])
			{
			case 3:
				if (3 == counts[1] && 3 == counts[2] && 3 == counts[3] && 3 == counts[4] && 3 == counts[5] && 3 == counts[6] && 3 == counts[7] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[21].point] + 7 == this->values[hand.cards[0].point])
				{
					hand.type = Type::TripleStraight; // 飞机（不带翅膀）
					return true;
				}
				else
					return false;
			case 2:
				if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5] && 2 == counts[6] && 2 == counts[7] && 2 == counts[8] && 2 == counts[9] && 2 == counts[10] && 2 == counts[11] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[22].point] + 11 == this->values[hand.cards[0].point])
				{
					hand.type = Type::PairStraight; // 连对
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 25:
			if (3 == counts[0] && 3 == counts[1] && 3 == counts[2] && 3 == counts[3] && 3 == counts[4] && 2 == counts[5] && 2 == counts[6] && 2 == counts[7] && 2 == counts[8] && 2 == counts[9] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point] && (JOKER_POINT != hand.cards[15].point || hand.cards[15].suit == hand.cards[16].suit))
			{
				hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
				return true;
			}
			else
				return false;
		case 27:
			if (3 == counts[0] && 3 == counts[1] && 3 == counts[2] && 3 == counts[3] && 3 == counts[4] && 3 == counts[5] && 3 == counts[6] && 3 == counts[7] && 3 == counts[8] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[24].point] + 8 == this->values[hand.cards[0].point])
			{
				hand.type = Type::TripleStraight; // 飞机（不带翅膀）
				return true;
			}
			else
				return false;
		case 30:
			if (3 == counts[1] && 3 == counts[2] && 3 == counts[3] && 3 == counts[4] && 3 == counts[5])
				switch (counts[6])
				{
				case 3:
					if (3 == counts[6] && 3 == counts[7] && 3 == counts[8] && 3 == counts[9] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[27].point] + 9 == this->values[hand.cards[0].point])
					{
						hand.type = Type::TripleStraight; // 飞机（不带翅膀）
						return true;
					}
					else
						return false;
				case 2:
					if (2 == counts[6] && 2 == counts[7] && 2 == counts[8] && 2 == counts[9] && 2 == counts[10] && 2 == counts[11] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[15].point] + 5 == this->values[hand.cards[0].point] && (JOKER_POINT != hand.cards[18].point || hand.cards[18].suit == hand.cards[19].suit))
					{
						hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
						return true;
					}
					else
						return false;
				default:
					return false;
				}
			else
				return false;
		case 33:
			if (3 == counts[0] && 3 == counts[1] && 3 == counts[2] && 3 == counts[3] && 3 == counts[4] && 3 == counts[5] && 3 == counts[6] && 3 == counts[7] && 3 == counts[8] && 3 == counts[9] && 3 == counts[10] && this->values[hand.cards[0].point] <= 12 && this->values[hand.cards[30].point] + 10 == this->values[hand.cards[0].point])
			{
				hand.type = Type::TripleStraight; // 飞机（不带翅膀）
				return true;
			}
			else
				return false;
		default:
			return false;
		}
	}
	virtual const bool processBasis(const Hand& hand) override final
	{
		if (Status::Assigned <= this->status && this->status <= Status::Started && this->amounts.size() == 1)
		{
			switch (hand.type)
			{
			case Type::Sextuple:
			case Type::Septuple:
				this->amounts[0] <<= 1;
				break;
			case Type::Octuple:
			case Type::QuadrupleJokers:
				this->amounts[0] += this->amounts[0] << 1;
				break;
			default:
				break;
			}
			return true;
		}
		else
			return false;
	}
	virtual const bool computeAmounts() override final
	{
		const size_t recordCount = this->records.size(), playerCount = this->players.size();
		if (Status::Over == this->status && recordCount >= 2 && 4 == playerCount)
			switch (this->amounts.size())
			{
			case 1:
			{
				/* Winner fetching */
				Player winner = INVALID_PLAYER;
				char playerFlags[4] = { -1, -1, -1, -1 };
				for (Player player = 0; player < 4; ++player)
					if (this->players[player].empty())
						if (INVALID_PLAYER == winner)
						{
							winner = player;
							playerFlags[player] = 3;
						}
						else
							return false;
				if (INVALID_PLAYER == winner)
					return false;
				else if (winner != this->dealer)
					for (Player player = 0; player < 4; ++player)
						if (player != this->dealer)
							playerFlags[player] = 1;
				
				/* Spring and anti-sprint parsing */
				bool isSpring = true, isAntiSpring = true;
				{
					const size_t handCount = this->records[1].size();
					for (size_t innerIdx = 1; innerIdx < handCount; ++innerIdx)
						if (Type::Single <= this->records[1][innerIdx].type && this->records[1][innerIdx].type <= Type::Octuple && !this->records[1][innerIdx].cards.empty())
							if (this->records[1][innerIdx].player == this->dealer)
								isAntiSpring = false;
							else
								isSpring = false;
				}
				for (size_t outerIdx = 2; outerIdx < recordCount && (isSpring || isAntiSpring); ++outerIdx)
				{
					const size_t handCount = this->records[outerIdx].size();
					for (size_t innerIdx = 0; innerIdx < handCount; ++innerIdx)
						if (Type::Single <= this->records[outerIdx][innerIdx].type && this->records[outerIdx][innerIdx].type <= Type::Octuple && !this->records[outerIdx][innerIdx].cards.empty())
							if (this->records[outerIdx][innerIdx].player == this->dealer)
								isAntiSpring = false;
							else
								isSpring = false;
				}
				if (isSpring)
					if (isAntiSpring)
						return false;
					else
						this->amounts[0] <<= 1;
				else if (isAntiSpring)
					this->amounts[0] <<= 1;
				
				/* Amount finalization */
				const Amount base = this->amounts[0];
				this->amounts = vector<Amount>(4);
				for (Player player = 0; player < 4; ++player)
					this->amounts[player] = base * playerFlags[player];
			}
			case 4:
				return true;
			default:
				return false;
			}
		else
			return false;
	}
	virtual const bool isAbsolutelyLargest(const Hand& hand) const override final
	{
		return Type::QuadrupleJokers == hand.type;
	}
	const bool coverLastHand(const Hand& currentHand) const override final
	{
		if (this->lastHand && Type::Single <= this->lastHand.type && this->lastHand.type <= Type::Octuple && !this->lastHand.cards.empty() && Type::Single <= currentHand.type && currentHand.type <= Type::Octuple && !currentHand.cards.empty())
			switch (this->lastHand.type)
			{
			case Type::Single: // 单牌
			case Type::Pair: // 对子
				return currentHand.type >= Type::Quintuple || Type::Quadruple == currentHand.type || (currentHand.type == this->lastHand.type && (JOKER_POINT == currentHand.cards[0].point && JOKER_POINT == this->lastHand.cards[0].point ? currentHand.cards[0].suit > this->lastHand.cards[0].suit : this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point]));
			case Type::SingleStraight: // 顺子
			case Type::PairStraight: // 连对
			case Type::Triple: // 三条
			case Type::TripleWithPair: // 三带一对
			case Type::TripleStraight: // 飞机（不拖不带）
			case Type::TripleStraightWithPairs: // 飞机（带对子）
				return currentHand.type >= Type::Quintuple || Type::Quadruple == currentHand.type || (currentHand.type == this->lastHand.type && currentHand.cards.size() == this->lastHand.cards.size() && this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point]);
			case Type::Quadruple: // 四条炸弹
				return currentHand.type >= Type::Quintuple || (Type::Quadruple == currentHand.type && this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point]);
			case Type::QuadrupleJokers: // 天王炸弹
			case Type::Quintuple: // 五张炸弹
			case Type::Sextuple: // 六张炸弹
			case Type::Septuple: // 七张炸弹
			case Type::Octuple: // 八张炸弹
				return currentHand.type > this->lastHand.type || (currentHand.type == this->lastHand.type && this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point]);
			default:
				return false;
			}
		else
			return false;
	}
	const string getBasisString() const
	{
		if (this->amounts.size() == 1)
		{
			char buffer[21] = { 0 };
			snprintf(buffer, 21, "%lld", this->amounts[0]);
			return (string)"当前倍数：" + buffer;
		}
		else
			return "";
	}
	const string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			Count callerAndRobberCount = 0;
			const size_t length = this->records[0].size();
			for (size_t idx = 0; idx < length; ++idx)
				if (this->records[0][idx].cards.size() == 1 && this->records[0][idx].cards[0].point)
					++callerAndRobberCount;
			char playerBuffer[4] = { 0 };
			if (0 == callerAndRobberCount && this->records[0].size() == 4)
			{
				snprintf(playerBuffer, 4, "%d", (this->records[0][0].player + 1));
				return "无人叫地主，强制玩家 " + (string)playerBuffer + " 为地主。";
			}
			else
			{
				string preRoundString{};
				for (const Hand& hand : this->records[0])
				{
					snprintf(playerBuffer, 4, "%d", hand.player + 1);
					switch (hand.cards.size())
					{
					case 0:
						preRoundString += "不叫（玩家 " + (string)playerBuffer + "） -> ";
						break;
					case 1:
						switch (hand.cards[0].point)
						{
						case 0:
							preRoundString += "不叫（玩家 " + (string)playerBuffer + "） -> ";
							break;
						case 1:
						case 2:
						case 3:
						{
							char scoreBuffer[4] = { 0 };
							snprintf(scoreBuffer, 4, "%d", hand.cards[0].point);
							preRoundString += (string)scoreBuffer + "分（玩家 " + playerBuffer + "） -> ";
							break;
						}
						default:
							return "预备回合信息检验异常。";
						}
						break;
					default:
						return "预备回合信息检验异常。";
					}
				}
				preRoundString.erase(preRoundString.length() - 4, 4);
				return preRoundString;
			}
		}
	}
	
public:
	Landlords4P() : PokerGame()
	{
		this->pokerType = "四人斗地主";
	}
	const bool initialize() override final
	{
		if (this->status >= Status::Ready)
		{
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->values.set(JOKER_POINT, value++);
			this->players = vector<vector<Card>>(4);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->dealer = INVALID_PLAYER;
			this->lastHand = Hand{};
			this->amounts.clear();
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
	const bool initialize(const size_t playerCount) override final { return 3 == playerCount && this->initialize(); }
	const bool deal() override final
	{
		if (this->status >= Status::Initialized)
		{
			this->deck.clear();
			this->add54CardsToDeck();
			this->add54CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < 4; ++player)
			{
				this->players[player] = vector<Card>(25);
				for (size_t idx = 0; idx < 25; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	const bool setLandlord(const Score score) override final
	{
		const Point point = static_cast<Point>(score);
		if (Status::Dealt == this->status && this->records.size() == 1 && 0 <= this->currentPlayer && this->currentPlayer < this->players.size())
			switch (this->records[0].size())
			{
			case 0:
				switch (score)
				{
				case Score::None:
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{} });
					this->nextPlayer();
					return true;
				case Score::One:
				case Score::Two:
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{ Card{ point } } });
					this->nextPlayer();
					this->lastHand = this->records[0][0];
					return true;
				case Score::Three:
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{ Card{ point } } });
					this->dealer = this->currentPlayer;
					this->lastHand = Hand{};
					this->amounts = vector<Amount>{ 3 };
					this->status = Status::Assigned;
					return true;
				default:
					return false;
				}
			case 1:
			case 2:
				switch (score)
				{
				case Score::None:
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{} });
					this->nextPlayer();
					return true;
				case Score::One:
				case Score::Two:
					if (!this->lastHand || (this->lastHand.cards.size() == 1 && point > this->lastHand.cards[0].point))
					{
						this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{ Card{ point } } });
						this->nextPlayer();
						this->lastHand = this->records[0].back();
						return true;
					}
					else
						return false;
				case Score::Three:
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{ Card{ point } } });
					this->dealer = this->currentPlayer;
					this->lastHand = Hand{};
					this->amounts = vector<Amount>{ 3 };
					this->status = Status::Assigned;
					return true;
				default:
					return false;
				}
			case 3:
				switch (score)
				{
				case Score::None:
					if (this->lastHand)
					{
						if (this->lastHand.cards.size() == 1)
						{
							this->currentPlayer = this->lastHand.player;
							this->amounts = vector<Amount>{ this->lastHand.cards[0].point };
						}
						else
							return false;
					}
					else
						this->currentPlayer = this->records[0][0].player;
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{} });
					break;
				case Score::One:
				case Score::Two:
					if (!this->lastHand || (this->lastHand.cards.size() == 1 && point > this->lastHand.cards[0].point))
					{
						this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{ Card{ point } } });
						this->amounts = vector<Amount>{ this->lastHand.cards[0].point };
						break;
					}
					else
						return false;
				case Score::Three:
					this->records[0].push_back(Hand{ this->currentPlayer, vector<Card>{ Card{ point } } });
					this->amounts = vector<Amount>{ 3 };
					break;
				default:
					return false;
				}
				this->dealer = this->currentPlayer;
				this->lastHand = Hand{};
				this->status = Status::Assigned;
				return true;
			default:
				return false;
			}
		else
			return false;
	}
	const bool display(const vector<Player>& selectedPlayers) const override final
	{
		return this->status >= Status::Assigned ? PokerGame::display(selectedPlayers, "地主", "地主牌：" + this->cards2string(this->deck, "", " | ", "（已公开）", "（空）") + "\n\n") : PokerGame::display(selectedPlayers, "拥有明牌", "地主牌：" + this->cards2string(this->deck, "", " | ", "（未公开）", "（空）") + "\n\n");
	}
};

class BigTwo : public PokerGame /* Previous: Landlords4P | Next: ThreeTwoOne */
{
private:
	const bool processHand(Hand& hand, vector<Candidate>& candidates) const override final
	{
		hand.type = Type::Invalid;
		candidates.clear();
		vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (this->values[card.point])
				++counts[card.point];
			else
				return false;
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && valueA > valueB) || (countA == countB && valueA == valueB && a.suit > b.suit); });
		if (adjacent_find(hand.cards.begin(), hand.cards.end()) != hand.cards.end())
			return false;
		sort(counts.begin(), counts.end(), [](const Count a, const Count b) { return a > b; });
		if (counts[0] > 4)
			return false;
		switch (hand.cards.size())
		{
		case 0:
			hand.type = Type::Empty;
			return true;
		case 1:
			hand.type = Type::Single;
			return true;
		case 2:
			if (2 == counts[0])
			{
				hand.type = Type::Pair;
				return true;
			}
			else
				return false;
		case 3:
			if (3 == counts[0])
			{
				hand.type = Type::Triple;
				return true;
			}
			else
				return false;
		case 5:
			switch (counts[0])
			{
			case 4: // && 1 == counts[1]
				hand.type = Type::QuadrupleWithSingle;
				return true;
			case 3:
				if (2 == counts[1])
				{
					hand.type = Type::TripleWithPair;
					return true;
				}
				else
					return false;
			case 1: // && 1 == counts[1] && 1 == counts[2] && 1 == counts[3] && 1 == counts[4]
			{
				const bool isSingleStraight = this->judgeStraight(hand.cards, 1, 2, false), isFlush = hand.cards[0].suit == hand.cards[1].suit && hand.cards[1].suit == hand.cards[2].suit && hand.cards[2].suit == hand.cards[3].suit && hand.cards[3].suit == hand.cards[4].suit;
				if (isSingleStraight)
					hand.type = isFlush ? Type::SingleFlushStraight : Type::SingleStraight;
				else if (isFlush)
					hand.type = Type::SingleFlush;
				else
					return false;
				return true;
			}
			default:
				return false;
			}
		default:
			return false;
		}
	}
	virtual const bool computeAmounts() override final
	{
		if (Status::Over == this->status)
			switch (this->amounts.size())
			{
			case 0:
			{
				if (this->players.size() != 4)
					return false;
				Count winnerCount = 0;
				this->amounts = vector<Amount>(4);
				for (size_t idx = 0; idx < 4; ++idx)
				{
					const size_t n = this->players[idx].size();
					if (n <= 0)
						++winnerCount;
					else if (n < 8)
						this->amounts[idx] = n;
					else if (8 <= n && n < 10)
						this->amounts[idx] = static_cast<Amount>(n) << 1;
					else if (10 <= n && n < 13)
						this->amounts[idx] = static_cast<Amount>(n) * 3;
					else if (13 == n)
						this->amounts[idx] = 52;
					else
					{
						this->amounts.clear();
						return false;
					}
					if (find(this->players[idx].begin(), this->players[idx].end(), Card{ 2, Suit::Spade }) != this->players[idx].end())
						this->amounts[idx] += 50;
					if (find(this->players[idx].begin(), this->players[idx].end(), Card{ 2, Suit::Heart }) != this->players[idx].end())
						this->amounts[idx] += 30;
					if (find(this->players[idx].begin(), this->players[idx].end(), Card{ 2, Suit::Club }) != this->players[idx].end())
						this->amounts[idx] += 20;
					if (find(this->players[idx].begin(), this->players[idx].end(), Card{ 2, Suit::Diamond }) != this->players[idx].end())
						this->amounts[idx] += 10;
				}
				if (1 == winnerCount)
				{
					const Amount s = this->amounts[0] + this->amounts[1] + this->amounts[2] + this->amounts[3];
					for (size_t idx = 0; idx < 4; ++idx)
						this->amounts[idx] = s - (this->amounts[idx] << 2);
				}
				else
				{
					this->amounts.clear();
					return false;
				}
			}
			case 4:
				return true;
			default:
				return false;
			}
		else
			return false;
	}
	virtual const bool isAbsolutelyLargest(const Hand& hand) const override final
	{
		return (Type::Single == hand.type || Type::Pair == hand.type || Type::Triple == hand.type || Type::Quadruple == hand.type || Type::SingleFlushStraight == hand.type) && (!hand.cards.empty() && Card { 2, Suit::Spade } == hand.cards[0]);
	}
	const bool coverLastHand(const Hand& currentHand) const override final
	{
		if (this->lastHand && Type::Single <= this->lastHand.type && this->lastHand.type <= Type::QuadrupleWithSingle && !this->lastHand.cards.empty() && Type::Single <= currentHand.type && currentHand.type <= Type::QuadrupleWithSingle && !currentHand.cards.empty())
			switch (this->lastHand.type)
			{
			case Type::Single: // 单牌
			case Type::SingleFlushStraight: // 一条龙|同花顺（长度只能为 5）
			case Type::Pair: // 对子
			case Type::Triple: // 三条
				return currentHand.type == this->lastHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit));
			case Type::SingleStraight: // 顺子（长度只能为 5）：可被一条龙|同花顺、金刚、葫芦/俘虏/副路、同花以及比自己大的顺子盖过
				return Type::SingleFlushStraight == currentHand.type || Type::QuadrupleWithSingle == currentHand.type || Type::TripleWithPair == currentHand.type || Type::SingleFlush == currentHand.type || (Type::SingleStraight == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::SingleFlush: // 同花（长度只能为 5）：可被一条龙|同花顺、金刚、葫芦/俘虏/副路以及比自己大的同花盖过
				return Type::SingleFlushStraight == currentHand.type || Type::QuadrupleWithSingle == currentHand.type || Type::TripleWithPair == currentHand.type || (Type::SingleFlush == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::TripleWithPair: // 葫芦/俘虏/副路：可被一条龙|同花顺、金刚、以及比自己大的葫芦/俘虏/副路盖过
				return Type::SingleFlushStraight == currentHand.type || Type::QuadrupleWithSingle == currentHand.type || (Type::TripleWithPair == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::QuadrupleWithSingle: // 金刚：可被一条龙|同花顺和比自己大的金刚盖过
				return Type::SingleFlushStraight == currentHand.type || (Type::QuadrupleWithSingle == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			default:
				return false;
			}
		else
			return false;
	}
	const string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString{};
			char playerBuffer[4] = { 0 };
			if (this->records[0].back().cards.size() == 1 && 1 == this->values[this->records[0].back().cards[0].point] && Suit::Diamond == this->records[0].back().cards[0].suit)
			{
				snprintf(playerBuffer, 4, "%d", this->records[0].back().player + 1);
				preRoundString = "玩家 " + (string)playerBuffer + " 拥有最小的牌（" + (string)this->records[0].back().cards[0] + "），拥有发牌权。";
			}
			else
				preRoundString = "预备回合信息检验异常。";
			return preRoundString;
		}
	}
	
public:
	BigTwo() : PokerGame()
	{
		this->pokerType = "锄大地";
	}
	const bool initialize() override final
	{
		if (this->status >= Status::Ready)
		{
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->players = vector<vector<Card>>(4);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->dealer = INVALID_PLAYER;
			this->lastHand = Hand{};
			this->amounts.clear();
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
	const bool initialize(const size_t playerCount) override final { return 3 == playerCount && this->initialize(); }
	const bool deal() override final
	{
		if (this->status >= Status::Initialized)
		{
			this->deck.clear();
			this->add52CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < 4; ++player)
			{
				this->players[player] = vector<Card>(13);
				for (size_t idx = 0; idx < 13; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	const bool display(const vector<Player>& selectedPlayers) const override final
	{
		return PokerGame::display(selectedPlayers, "方块3 先出", "");
	}
};

class ThreeTwoOne : public PokerGame /* Previous: BigTwo | Next: Wuguapi */
{
private:
	const bool processHand(Hand& hand, vector<Candidate>& candidates) const override final
	{
		hand.type = Type::Invalid;
		vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (this->values[card.point])
				++counts[card.point];
			else
			{
				candidates.clear();
				return false;
			}
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && valueA > valueB) || (countA == countB && valueA == valueB && a.suit > b.suit); });
		if (adjacent_find(hand.cards.begin(), hand.cards.end()) != hand.cards.end())
		{
			candidates.clear();
			return false;
		}
		sort(counts.begin(), counts.end(), [](const Count a, const Count b) { return a > b; });
		if (counts[0] > 4)
		{
			candidates.clear();
			return false;
		}
		const size_t cardCount = hand.cards.size();
		if (6 == cardCount)
			switch (counts[0])
			{
			case 4:
				candidates.clear();
				if (2 == counts[1])
				{
					rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end());
					hand.type = Type::TripleWithPairSingle; // 三两一
					return true;
				}
				else
					return false;
			case 3:
				switch (counts[1])
				{
				case 3:
				{
					vector<Candidate> potentialHands{};
					char buffers[2][3] = { { 0 } };
					snprintf(buffers[0], 3, "%d", hand.cards[0].point);
					snprintf(buffers[1], 3, "%d", hand.cards[3].point);
					if (this->values[hand.cards[3].point] + 1 == this->values[hand.cards[0].point])
					{
						potentialHands.emplace_back(hand, (string)"解析为不拖不带、长度为 2 且点数为 " + buffers[0] + " 的三顺（``Type::TripleStraight``）");
						potentialHands.back().hand.type = Type::TripleStraight; // 三顺
					}
					potentialHands.emplace_back(hand, (string)"解析为三条 " + buffers[0] + " 的三两一，两为一对 " + buffers[1] + "，一为一张 " + buffers[1] + "（``Type::TripleWithPairSingle``）");
					potentialHands.back().hand.type = Type::TripleWithPairSingle; // 三两一
					rotate(hand.cards.begin(), hand.cards.begin() + 3, hand.cards.end());
					potentialHands.emplace_back(hand, (string)"解析为三条 " + buffers[1] + " 的三两一，两为一对 " + buffers[0] + "，一为一张 " + buffers[0] + "（``Type::TripleWithPairSingle``）");
					potentialHands.back().hand.type = Type::TripleWithPairSingle; // 三两一
					if (3 == hand.cards[0].point && 2 == hand.cards[3].point)
					{
						potentialHands.emplace_back(hand, (string)"解析为不拖不带、长度为 2 且点数为 " + buffers[1] + " 的三顺（``Type::TripleStraight``）");
						potentialHands.back().hand.type = Type::TripleStraight; // 三顺
					}
					if (this->lastHand && hand.player != this->lastHand.player)
						for (vector<Candidate>::iterator it = potentialHands.begin(); it != potentialHands.end();)
							if (this->coverLastHand(it->hand))
								++it;
							else
								it = potentialHands.erase(it);
					switch (potentialHands.size())
					{
					case 0:
						candidates.clear();
						return false;
					case 1:
						hand = move(potentialHands[0].hand);
						candidates.clear();
						return true;
					default:
						if (candidates.size() == 1)
						{
							const vector<Candidate>::iterator it = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
							if (it != potentialHands.end())
							{
								hand = move(it->hand);
								candidates.clear();
								return true;
							}
						}
						candidates = move(potentialHands);
						return false;
					}
				}
				case 2: // && 1 == counts[2]
				{
					hand.type = Type::TripleWithPairSingle; // 三两一
					candidates.clear();
					return true;
				}
				default:
					return false;
				}
			case 2:
				if (this->judgeStraight(hand.cards, 2, 3, true))
				{
					hand.type = Type::PairStraight; // 连对
					candidates.clear();
					return true;
				}
				else
					return false;
			case 1:
				if (this->judgeStraight(hand.cards, 1, 3, true))
				{
					hand.type = Type::SingleStraight; // 顺子
					candidates.clear();
					return true;
				}
				else
					return false;
			default:
				candidates.clear();
				return false;
			}
		else
		{
			candidates.clear();
			switch (cardCount)
			{
			case 0:
				hand.type = Type::Empty; // 要不起
				return true;
			case 1:
				hand.type = Type::Single; // 单牌
				return true;
			case 2:
				if (2 == counts[0])
				{
					hand.type = Type::Pair; // 对子
					return true;
				}
				else
					return false;
			case 3:
				switch (counts[0])
				{
				case 3:
					hand.type = Type::Triple; // 三条
					return true;
				case 1:
					if (this->judgeStraight(hand.cards, 1, 3, true) == 1)
					{
						hand.type = Type::SingleStraight; // 顺子
						return true;
					}
					else
						return false;
				default:
					return false;
				}
			case 4:
				switch (counts[0])
				{
				case 4:
					hand.type = Type::Quadruple; // 四条
					return true;
				case 2:
					if (2 == counts[1] && this->judgeStraight(hand.cards, 2, 3, true))
					{
						hand.type = Type::PairStraight; // 连对
						return true;
					}
					else
						return false;
				case 1: // && 1 == counts[1] && 1 == counts[2] && 1 == counts[3]
					if (this->judgeStraight(hand.cards, 1, 3, true))
					{
						hand.type = Type::SingleStraight; // 顺子
						return true;
					}
					else
						return false;
				default:
					return false;
				}
			case 5:
				switch (counts[0])
				{
				case 4: // && 1 == counts[1]
					hand.type = Type::QuadrupleWithSingle; // 四夹一
					return true;
				case 3:
					if (2 == counts[1])
					{
						hand.type = Type::TripleWithPair; // 三两不一
						return true;
					}
					else
						return false;
				case 1: // && 1 == counts[2] && 1 == counts[3] && 1 == counts[4]
					if (this->judgeStraight(hand.cards, 1, 3, true))
					{
						hand.type = Type::SingleStraight; // 顺子
						return true;
					}
					else
						return false;
				default:
					return false;
				}
			case 7:
				switch (counts[0])
				{
				case 4:
					if (3 == counts[1])
					{
						vector<Card> bodyCards(hand.cards);
						bodyCards.erase(bodyCards.begin() + 3);
						if (this->judgeStraight(bodyCards, 3, 3, true))
						{
							bodyCards.push_back(hand.cards[3]);
							hand.cards = move(bodyCards);
							hand.type = Type::TripleStraightWithSingle; // 三顺夹一
							return true;
						}
						else
							return false;
					}
					else
						return false;
				case 3:
					switch (counts[1])
					{
					case 3: // && 1 == counts[2]
					{
						vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (this->judgeStraight(bodyCards, 3, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = bodyCards;
							hand.type = Type::TripleStraightWithSingle; // 三顺夹一
							return true;
						}
						else
							return false;
					}
					case 2:
						if (2 == counts[2])
						{
							vector<Card> bodyCards(hand.cards);
							bodyCards.erase(bodyCards.begin() + 2);
							if (this->judgeStraight(bodyCards, 2, 3, true))
							{
								bodyCards.push_back(hand.cards[2]);
								hand.cards = move(bodyCards);
								hand.type = Type::PairStraightWithSingle; // 连对夹一
								return true;
							}
							else
								return false;
						}
						else
							return false;
					default:
						return false;
					}
				case 2:
					if (2 == counts[1] && 2 == counts[2]) // && 1 == counts[3]
					{
						vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (this->judgeStraight(bodyCards, 2, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = bodyCards;
							hand.type = Type::PairStraightWithSingle; // 连对夹一
							return true;
						}
						else
							return false;
					}
				case 1:
					if (this->judgeStraight(hand.cards, 1, 3, true))
					{
						hand.type = Type::SingleStraight; // 顺子
						return true;
					}
					else
						return false;
				default:
					return false;
				}
			case 8:
				switch (this->judgeStraight(hand.cards, 3, true))
				{
				case 1:
					hand.type = Type::SingleStraight; // 顺子
					return true;
				case 2:
					hand.type = Type::PairStraight; // 连对
					return true;
				case 4:
					hand.type = Type::QuadrupleStraight; // 四顺
					return true;
				default:
					return false;
				}
			case 9:
				switch (counts[0])
				{
				case 4:
					if (4 == counts[1]) // && 1 == counts[2]
					{
						vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (this->judgeStraight(bodyCards, 4, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = move(bodyCards);
							hand.type = Type::QuadrupleStraightWithSingle; // 四顺夹一
							return true;
						}
						else
							return false;
					}
					else
						return false;
				case 3:
					switch (counts[1])
					{
					case 3:
						if (3 == counts[2] && this->judgeStraight(hand.cards, 3, true))
						{
							hand.type = Type::TripleStraight; // 三顺
							return true;
						}
					case 2:
						if (2 == counts[2] && 2 == counts[3])
						{
							vector<Card> bodyCards(hand.cards);
							bodyCards.erase(bodyCards.begin() + 2);
							if (this->judgeStraight(bodyCards, 2, 3, true))
							{
								bodyCards.push_back(hand.cards[2]);
								hand.cards = move(bodyCards);
								hand.type = Type::PairStraightWithSingle; // 连对夹一
								return true;
							}
							else
								return false;
						}
						else
							return false;
					default:
						return false;
					}
				case 2:
					if (2 == counts[1] && 2 == counts[2] && 2 == counts[3]) // && 1 == counts[4]
					{
						vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (this->judgeStraight(bodyCards, 2, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = move(bodyCards);
							hand.type = Type::PairStraightWithSingle; // 连对夹一
							return true;
						}
						else
							return false;
					}
					else
						return false;
				case 1: // && 1 == counts[1] && ... && 1 == counts[8]
					if (this->judgeStraight(hand.cards, 1, 3, true))
					{
						hand.type = Type::SingleStraight; // 顺子
						return true;
					}
					else
						return false;
				default:
					return false;
				}
			case 10:
				switch (counts[0])
				{
				case 4:
					if (3 == counts[1] && 3 == counts[2])
					{
						vector<Card> bodyCards(hand.cards);
						bodyCards.erase(bodyCards.begin() + 3);
						if (this->judgeStraight(bodyCards, 3, 3, true))
						{
							bodyCards.push_back(hand.cards[3]);
							hand.cards = move(bodyCards);
							hand.type = Type::TripleStraightWithSingle; // 三顺夹一
							return true;
						}
						else
							return false;
					}
					else
						return false;
				case 3:
					if (3 == counts[1] && 3 == counts[2]) // && 1 == counts[3]
					{
						vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (this->judgeStraight(bodyCards, 3, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = move(bodyCards);
							hand.type = Type::TripleStraightWithSingle; // 三顺夹一
							return true;
						}
						else
							return false;
					}
					else
						return false;
				case 2:
					if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4])
					{
						vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (this->judgeStraight(bodyCards, 2, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = move(bodyCards);
							hand.type = Type::PairStraight; // 连对
							return true;
						}
						else
							return false;
					}
					else
						return false;
				case 1: // && 1 == counts[1] && ... && 1 == counts[9]
					if (this->judgeStraight(hand.cards, 1, 3, true))
					{
						hand.type = Type::SingleStraight; // 顺子
						return true;
					}
					else
						return false;
				default:
					return false;
				}
			case 11:
				switch (counts[0])
				{
				case 3:
					if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4])
					{
						vector<Card> bodyCards(hand.cards);
						bodyCards.erase(bodyCards.begin() + 2);
						if (this->judgeStraight(bodyCards, 2, 3, true))
						{
							bodyCards.push_back(hand.cards[2]);
							hand.cards = move(bodyCards);
							hand.type = Type::PairStraightWithSingle; // 连对夹一
							return true;
						}
						else
							return false;
					}
					else
						return false;
				case 2:
					if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4]) // && 1 == counts[5]
					{
						vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (this->judgeStraight(bodyCards, 2, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = move(bodyCards);
							hand.type = Type::PairStraightWithSingle; // 连对夹一
							return true;
						}
						else
							return false;
					}
					else
						return false;
				case 1:
					if (this->judgeStraight(hand.cards, 1, 3, true))
					{
						hand.type = Type::SingleStraight; // 顺子
						return true;
					}
					else
						return false;
				default:
					return false;
				}
			case 12:
				switch (this->judgeStraight(hand.cards, 3, true))
				{
				case 1:
					hand.type = Type::SingleStraight; // 顺子
					return true;
				case 2:
					hand.type = Type::PairStraight; // 连对
					return true;
				case 3:
					hand.type = Type::TripleStraight; // 三顺
					return true;
				case 4:
					hand.type = Type::QuadrupleStraight; // 四顺
					return true;
				default:
					return false;
				}
			case 13:
				switch (counts[0])
				{
				case 4:
					if (3 == counts[1] && 3 == counts[2] && 3 == counts[3])
					{
						vector<Card> bodyCards(hand.cards);
						bodyCards.erase(bodyCards.begin() + 3);
						if (this->judgeStraight(bodyCards, 3, 3, true))
						{
							bodyCards.push_back(hand.cards[3]);
							hand.cards = move(bodyCards);
							hand.type = Type::TripleStraightWithSingle; // 三顺夹一
							return true;
						}
						else
							return false;
					}
					else
						return false;
				case 3:
					switch (counts[1])
					{
					case 3:
						if (3 == counts[2] && 3 == counts[3]) // && 1 == counts[4]
						{
							vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
							if (this->judgeStraight(bodyCards, 3, 3, true))
							{
								bodyCards.push_back(hand.cards.back());
								hand.cards = move(bodyCards);
								hand.type = Type::TripleStraightWithSingle; // 三顺夹一
								return true;
							}
							else
								return false;
						}
						else
							return false;
					case 2:
						if (2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5])
						{
							vector<Card> bodyCards(hand.cards);
							bodyCards.erase(bodyCards.begin() + 2);
							if (this->judgeStraight(bodyCards, 2, 3, true))
							{
								bodyCards.push_back(hand.cards[2]);
								hand.cards = move(bodyCards);
								hand.type = Type::PairStraightWithSingle; // 连对夹一
								return true;
							}
							else
								return false;
						}
						else
							return false;
					default:
						return false;
					}
				case 2:
					if (2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5]) // && 1 == counts[6]
					{
						vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (2 == this->judgeStraight(bodyCards, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = move(bodyCards);
							hand.type = Type::PairStraightWithSingle; // 连对夹一
							return true;
						}
						else
							return false;
					}
					else
						return false;
				case 1:
					if (this->judgeStraight(hand.cards, 1, 3, true))
					{
						hand.type = Type::SingleStraight; // 顺子
						return true;
					}
					else
						return false;
				default:
					return false;
				}
			default:
				return false;
			}
		}
	}
	virtual const bool computeAmounts() override final
	{
		if (Status::Over == this->status)
			switch (this->amounts.size())
			{
			case 0:
			{
				if (this->players.size() != 4)
					return false;
				Player winner = INVALID_PLAYER;
				this->amounts = vector<Amount>(4);
				Amount s = 0;
				for (size_t idx = 0; idx < 4; ++idx)
				{
					switch (this->players[idx].size())
					{
					case 0:
						if (INVALID_PLAYER == winner)
							winner = static_cast<Player>(idx);
						else
						{
							this->amounts.clear();
							return false;
						}
						break;
					case 1:
						break;
					case 2:
					case 3:
					case 4:
					case 5:
						this->amounts[idx] = -1;
						break;
					case 6:
					case 7:
					case 8:
					case 9:
						this->amounts[idx] = -2;
						break;
					case 10:
					case 11:
					case 12:
						this->amounts[idx] = -3;
						break;
					case 13:
						this->amounts[idx] = -5;
						break;
					default:
						this->amounts.clear();
						return false;
					}
					s -= this->amounts[idx];
				}
				if (INVALID_PLAYER == winner)
				{
					this->amounts.clear();
					return false;
				}
				else
					this->amounts[winner] = s;
			}
			case 4:
				return true;
			default:
				this->amounts.clear();
				return false;
			}
		else
			return false;
	}
	virtual const bool isAbsolutelyLargest(const Hand& hand) const override final
	{
		return Type::Single <= hand.type && hand.type <= Type::QuadrupleStraightWithSingle && !hand.cards.empty() && 2 == hand.cards[0].point;
	}
	const bool coverLastHand(const Hand& currentHand) const override final
	{
		if (this->lastHand && Type::Single <= this->lastHand.type && this->lastHand.type <= Type::QuadrupleStraightWithSingle && !this->lastHand.cards.empty() && Type::Single <= currentHand.type && currentHand.type <= Type::QuadrupleStraightWithSingle && !currentHand.cards.empty())
			return (currentHand.type == this->lastHand.type || (Type::TripleWithPairSingle == this->lastHand.type && Type::TripleStraight == currentHand.type)) && currentHand.cards.size() == this->lastHand.cards.size() && this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point];
		else
			return false;
	}
	const string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString{};
			char playerBuffer[4] = { 0 };
			if (this->records[0].back().cards.size() == 1 && 1 == this->values[this->records[0].back().cards[0].point] && Suit::Diamond == this->records[0].back().cards[0].suit)
			{
				snprintf(playerBuffer, 4, "%d", this->records[0].back().player + 1);
				preRoundString = "玩家 " + (string)playerBuffer + " 拥有最小的牌（" + (string)this->records[0].back().cards[0] + "），拥有发牌权。";
			}
			else
				preRoundString = "预备回合信息检验异常。";
			return preRoundString;
		}
	}
	
public:
	ThreeTwoOne() : PokerGame()
	{
		this->pokerType = "三两一";
	}
	const bool initialize() override final
	{
		if (this->status >= Status::Ready)
		{
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->players = vector<vector<Card>>(4);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->dealer = INVALID_PLAYER;
			this->lastHand = Hand{};
			this->amounts.clear();
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
	const bool initialize(const size_t playerCount) override final { return 3 == playerCount && this->initialize(); }
	const bool deal() override final
	{
		if (this->status >= Status::Initialized)
		{
			this->deck.clear();
			this->add52CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < 4; ++player)
			{
				this->players[player] = vector<Card>(13);
				for (size_t idx = 0; idx < 13; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	const bool nextPlayer() override final
	{
		if (this->records.empty() || this->currentPlayer >= 4)
			return false;
		else
		{
			bool flags[4] = { true, true, true, true };
			for (const Hand& hand : this->records.back())
				if (hand.player >= 4)
					return false;
				else if (Type::Empty == hand.type)
					flags[hand.player] = false;
			Player offsetPlayer = this->currentPlayer;
			for (Count count = 0; count < 4; ++count)
			{
				offsetPlayer = (offsetPlayer + 1) % 4;
				if (flags[offsetPlayer])
				{
					this->currentPlayer = offsetPlayer;
					return true;
				}
			}
			return false;
		}
	}
	const bool display(const vector<Player>& selectedPlayers) const override final
	{
		return PokerGame::display(selectedPlayers, "方块3 先出", "");
	}
};

class Wuguapi : public PokerGame /* Previous: ThreeTwoOne | Next: Qiguiwueryi */
{
private:
	const bool processHand(Hand& hand, vector<Candidate>& candidates) const override final
	{
		hand.type = Type::Invalid;
		candidates.clear();
		bool littleJoker = false, bigJoker = false;
		vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (JOKER_POINT == card.point)
				switch (card.suit)
				{
				case Suit::Black:
					if (littleJoker)
						return false;
					else
						littleJoker = true;
				case Suit::Red:
					if (bigJoker)
						return false;
					else
						bigJoker = true;
				default:
					return false;
				}
			else if (this->values[card.point])
				++counts[card.point];
			else
				return false;
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && valueA > valueB) || (countA == countB && valueA == valueB && a.suit > b.suit); });
		if (adjacent_find(hand.cards.begin(), hand.cards.end()) != hand.cards.end())
			return false;
		sort(counts.begin(), counts.end(), [](const Count a, const Count b) { return a > b; });
		if (counts[0] > 4)
			return false;
		switch (hand.cards.size())
		{
		case 0:
			hand.type = Type::Empty;
			return true;
		case 1:
			hand.type = Type::Single;
			return true;
		case 2:
			if (2 == counts[0])
			{
				hand.type = JOKER_POINT == hand.cards[0].point ? Type::PairJokers : Type::Pair;
				return true;
			}
			else
				return false;
		case 3:
			switch (counts[0])
			{
			case 3:
				hand.type = Type::Triple;
				return true;
			case 1:
				if (1 == counts[1]) // && 1 == counts[2]
				{
					const bool isSingleStraight = this->judgeStraight(hand.cards, 1, 2, false), isFlush = hand.cards[0].suit == hand.cards[1].suit && hand.cards[1].suit == hand.cards[2].suit;
					if (isSingleStraight)
						hand.type = isFlush ? Type::SingleFlushStraight : Type::SingleStraight;
					else if (isFlush)
						hand.type = Type::SingleFlush;
					else
						return false;
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 4:
			switch (counts[0])
			{
			case 4:
				hand.type = Type::Quadruple;
				return true;
			case 1:
				if (1 == counts[1]) // && 1 == counts[2] && 1 == counts[3]
				{
					const bool isSingleStraight = this->judgeStraight(hand.cards, 1, 2, false), isFlush = hand.cards[0].suit == hand.cards[1].suit && hand.cards[1].suit == hand.cards[2].suit && hand.cards[2] == hand.cards[3];
					if (isSingleStraight)
						hand.type = isFlush ? Type::SingleFlushStraight : Type::SingleStraight;
					else if (isFlush)
						hand.type = Type::SingleFlush;
					else
						return false;
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		case 5:
			switch (counts[0])
			{
			case 4: // && 1 == counts[1]
				hand.type = Type::QuadrupleWithSingle;
				return true;
			case 3:
				if (2 == counts[1])
				{
					hand.type = Type::TripleWithPair;
					return true;
				}
				else
					return false;

			case 1:
				if (1 == counts[1]) // && 1 == counts[2] && 1 == counts[3]
				{
					const bool isSingleStraight = this->judgeStraight(hand.cards, 1, 2, false), isFlush = hand.cards[0].suit == hand.cards[1].suit && hand.cards[1].suit == hand.cards[2].suit && hand.cards[2] == hand.cards[3] && hand.cards[3] == hand.cards[4];
					if (isSingleStraight)
						hand.type = isFlush ? Type::SingleFlushStraight : Type::SingleStraight;
					else if (isFlush)
						hand.type = Type::SingleFlush;
					else
						return false;
					return true;
				}
				else
					return false;
			default:
				return false;
			}
		default:
			return false;
		}
	}
	const bool isOver() const override final
	{
		if (this->status >= Status::Started && this->deck.empty())
		{
			bool hasCards = false;
			for (const vector<Card>& cards : this->players)
				if (!cards.empty())
					if (hasCards)
						return false;
					else
						hasCards = true;
			return true;
		}
		return false;
	}
	const bool computeAmounts() override final
	{
		if (Status::Over == this->status)
		{
			if (this->amounts.size() != this->players.size())
			{

				/////
			}
			return true;
		}
		else
			return false;
	}
	virtual const bool isAbsolutelyLargest(const Hand& hand) const override final
	{
		return ((Type::Single == hand.type || Type::PairJokers == hand.type) && Card { JOKER_POINT, Suit::Red } == hand.cards[0]) || (Type::SingleFlushStraight == hand.type && Card{ 5, Suit::Spade } == hand.cards[0]);
	}
	const bool coverLastHand(const Hand& currentHand) const override final
	{
		if (this->lastHand && Type::Single <= this->lastHand.type && this->lastHand.type <= Type::QuadrupleWithSingle && !this->lastHand.cards.empty() && Type::Single <= currentHand.type && currentHand.type <= Type::QuadrupleWithSingle && !currentHand.cards.empty())
			switch (this->lastHand.type)
			{
			case Type::Single: // 单牌
				return currentHand.type == this->lastHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit));
			case Type::SingleStraight: // 顺子
				switch (this->lastHand.cards.size())
				{
				case 3: // 可被一条龙|同花顺、三条、同花以及比自己大的顺子盖过
					return currentHand.cards.size() == this->lastHand.cards.size() && (Type::SingleFlushStraight == currentHand.type || Type::Triple == currentHand.type || Type::SingleFlush == currentHand.type || (Type::SingleStraight == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit))));
				case 4: // 可被一条龙|同花顺、四条、同花以及比自己大的顺子盖过
					return currentHand.cards.size() == this->lastHand.cards.size() && (Type::SingleFlushStraight == currentHand.type || Type::Quadruple == currentHand.type || Type::SingleFlush == currentHand.type || (Type::SingleStraight == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit))));
				case 5: // 可被一条龙|同花顺、金刚、葫芦/俘虏/副路、同花以及比自己大的顺子盖过
					return currentHand.cards.size() == this->lastHand.cards.size() && (Type::SingleFlushStraight == currentHand.type || Type::QuadrupleWithSingle == currentHand.type || Type::TripleWithPair == currentHand.type || Type::SingleFlush == currentHand.type || (Type::SingleStraight == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit))));
				default:
					return false;
				}
			case Type::SingleFlush: // 同花
				switch (this->lastHand.cards.size())
				{
				case 3: // 可被一条龙|同花顺、三条、以及比自己大的同花盖过
					return currentHand.cards.size() == this->lastHand.cards.size() && (Type::SingleFlushStraight == currentHand.type || Type::Triple == currentHand.type || (Type::SingleFlush == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit))));
				case 4: // 可被一条龙|同花顺、四条、以及比自己大的同花盖过
					return currentHand.cards.size() == this->lastHand.cards.size() && (Type::SingleFlushStraight == currentHand.type || Type::Quadruple == currentHand.type || (Type::SingleFlush == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit))));
				case 5: // 可被一条龙|同花顺、金刚、葫芦/俘虏/副路、以及比自己大的同花盖过
					return currentHand.cards.size() == this->lastHand.cards.size() && (Type::SingleFlushStraight == currentHand.type || Type::QuadrupleWithSingle == currentHand.type || Type::TripleWithPair == currentHand.type || (Type::SingleFlush == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit))));
				default:
					return false;
				}
			case Type::SingleFlushStraight: // 一条龙|同花顺
				return currentHand.type == this->lastHand.type && currentHand.cards.size() == this->lastHand.cards.size() && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit));
			case Type::Pair: // 对子
				return Type::PairJokers == currentHand.type || (currentHand.type == this->lastHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::PairJokers: // 对鬼
				return false;
			case Type::Triple: // 三条：可被一条龙|同花顺和比自己大的三条盖过
				return (Type::SingleFlushStraight == currentHand.type && currentHand.cards.size() == 3) || (Type::Triple == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::TripleWithPair: // 葫芦/俘虏/副路：可被一条龙|同花顺、金刚、以及比自己大的葫芦/俘虏/副路盖过
				return Type::SingleFlushStraight == currentHand.type || Type::QuadrupleWithSingle == currentHand.type || (Type::TripleWithPair == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::Quadruple: // 四条：可被一条龙|同花顺和比自己大的四条盖过
				return (Type::SingleFlushStraight == currentHand.type && currentHand.cards.size() == 4) || (Type::Quadruple == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::QuadrupleWithSingle: // 金刚：可被一条龙|同花顺和比自己大的金刚盖过
				return Type::SingleFlushStraight == currentHand.type || (Type::QuadrupleWithSingle == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			default:
				return false;
			}
		else
			return false;
	}
	const string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString{};
			char playerBuffer[4] = { 0 };
			for (const Hand& hand : this->records[0])
				if (hand.cards.size() == 1)
				{
					snprintf(playerBuffer, 4, "%d", hand.player + 1);
					preRoundString += (string)hand.cards[0] + "（玩家 " + playerBuffer + "） > ";
				}
				else
					return "预备回合信息检验异常。";
			preRoundString.erase(preRoundString.length() - 3, 3);
			return preRoundString;
		}
	}
	
public:
	Wuguapi() : PokerGame()
	{
		this->pokerType = "五瓜皮";
	}
	const bool initialize() override final { return this->initialize(2); }
	const bool initialize(const size_t playerCount) override final
	{
		if (this->status >= Status::Ready && 2 <= playerCount && playerCount <= 10)
		{
			Value value = 1;
			for (Point point = 6; point <= 13; ++point)
				this->values.set(point, value++);
			for (Point point = 1; point <= 5; ++point)
				this->values.set(point, value++);
			this->values.set(JOKER_POINT, value++);
			this->players = vector<vector<Card>>(playerCount);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->dealer = INVALID_PLAYER;
			this->lastHand = Hand{};
			this->amounts.clear();
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
	const bool deal() override final
	{
		if (this->status >= Status::Initialized)
		{
			this->deck.clear();
			this->add54CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			const size_t playerCount = this->players.size();
			for (Player player = 0; player < playerCount; ++player)
			{
				this->players[player] = vector<Card>(5);
				for (size_t idx = 0; idx < 5; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	const bool display(const vector<Player>& selectedPlayers) const override final
	{
		return PokerGame::display(selectedPlayers, "最小先出", "牌堆（自下往上）：" + this->cards2string(this->deck, "", " | ", "", "（空）") + "\n\n");
	}
};

class Qiguiwueryi : public PokerGame /* Previous: ThreeTwoOne | Next: Qiguiwuersan */
{
private:
	const bool processHand(Hand& hand, vector<Candidate>& candidates) const override final
	{
		hand.type = Type::Invalid;
		candidates.clear();
		bool littleJoker = false, bigJoker = false;
		vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (JOKER_POINT == card.point)
				switch (card.suit)
				{
				case Suit::Black:
					if (littleJoker)
						return false;
					else
						littleJoker = true;
				case Suit::Red:
					if (bigJoker)
						return false;
					else
						bigJoker = true;
				default:
					return false;
				}
			else if (this->values[card.point])
				++counts[card.point];
			else
				return false;
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && valueA > valueB) || (countA == countB && valueA == valueB && a.suit > b.suit); });
		if (adjacent_find(hand.cards.begin(), hand.cards.end()) != hand.cards.end())
			return false;
		sort(counts.begin(), counts.end(), [](const Count a, const Count b) { return a > b; });
		if (counts[0] > 4)
			return false;
		switch (hand.cards.size())
		{
		case 0:
			hand.type = Type::Empty;
			return true;
		case 1:
			hand.type = Type::Single;
			return true;
		case 2:
			if (2 == counts[0])
			{
				hand.type = JOKER_POINT == hand.cards[0].point ? Type::PairJokers : Type::Pair;
				return true;
			}
			else
				return false;
		case 3:
			if (3 == counts[0])
			{
				hand.type = Type::Triple;
				return true;
			}
			else
				return false;
		case 4:
			if (4 == counts[0])
			{
				hand.type = Type::Quadruple;
				return true;
			}
			else
				return false;
		default:
			return false;
		}
	}
	const bool isOver() const override final
	{
		if (this->status >= Status::Started && this->deck.empty())
		{
			bool hasCards = false;
			for (const vector<Card>& cards : this->players)
				if (!cards.empty())
					if (hasCards)
						return false;
					else
						hasCards = true;
			return true;
		}
		return false;
	}
	const bool computeAmounts() override final
	{
		if (Status::Over == this->status)
		{
			if (this->amounts.size() != this->players.size())
			{

				/////
			}
			return true;
		}
		else
			return false;
	}
	virtual const bool isAbsolutelyLargest(const Hand& hand) const override final
	{
		return (Type::Single == hand.type || Type::Pair == hand.type || Type::Triple == hand.type || Type::Quadruple == hand.type) && (!hand.cards.empty() && Card { 7, Suit::Spade } == hand.cards[0]);
	}
	const bool coverLastHand(const Hand& currentHand) const override final
	{
		if (this->lastHand && Type::Single <= this->lastHand.type && this->lastHand.type <= Type::Quadruple && !this->lastHand.cards.empty() && Type::Single <= currentHand.type && currentHand.type <= Type::Quadruple && !currentHand.cards.empty())
			switch (this->lastHand.type)
			{
			case Type::Single:
			case Type::Triple:
			case Type::Quadruple:
				return currentHand.type == this->lastHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit));
			case Type::Pair:
			case Type::PairJokers:
				return (Type::Pair == currentHand.type || Type::PairJokers == currentHand.type) && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit));
			default:
				return false;
			}
		else
			return false;
	}
	const string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString{};
			char playerBuffer[4] = { 0 };
			for (const Hand& hand : this->records[0])
				if (hand.cards.size() == 1)
				{
					snprintf(playerBuffer, 4, "%d", hand.player + 1);
					preRoundString += (string)hand.cards[0] + "（玩家 " + playerBuffer + "） > ";
				}
				else
					return "预备回合信息检验异常。";
			preRoundString.erase(preRoundString.length() - 3, 3);
			return preRoundString;
		}
	}
	
public:
	Qiguiwueryi() : PokerGame()
	{
		this->pokerType = "七鬼五二一";
	}
	const bool initialize() override { return this->initialize(2); }
	const bool initialize(const size_t playerCount) override
	{
		if (this->status >= Status::Ready && 2 <= playerCount && playerCount <= 7)
		{
			Value value = 1;
			this->values.set(3, value++);
			this->values.set(4, value++);
			this->values.set(6, value++);
			for (Point point = 8; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->values.set(5, value++);
			this->values.set(JOKER_POINT, value++);
			this->values.set(7, value++);
			this->players = vector<vector<Card>>(playerCount);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->dealer = INVALID_PLAYER;
			this->lastHand = Hand{};
			this->amounts.clear();
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
	const bool deal() override final
	{
		if (this->status >= Status::Initialized)
		{
			this->deck.clear();
			this->add54CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			const size_t playerCount = this->players.size();
			for (Player player = 0; player < playerCount; ++player)
			{
				this->players[player] = vector<Card>(7);
				for (size_t idx = 0; idx < 7; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	const bool display(const vector<Player>& selectedPlayers) const override final
	{
		return PokerGame::display(selectedPlayers, "最小先出", "牌堆（自下往上）：" + this->cards2string(this->deck, "", " | ", "", "（空）") + "\n\n");
	}
};

class Qiguiwuersan : public Qiguiwueryi /* Previous: Qiguiwueryi */
{
public:
	Qiguiwuersan() : Qiguiwueryi()
	{
		this->pokerType = "七鬼五二三";
	}
	const bool initialize() override final { return this->initialize(2); }
	const bool initialize(const size_t playerCount) override final
	{
		if (this->status >= Status::Ready && 2 <= playerCount && playerCount <= 7)
		{
			Value value = 1;
			this->values.set(1, value++);
			this->values.set(4, value++);
			this->values.set(6, value++);
			for (Point point = 8; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(3, value++);
			this->values.set(2, value++);
			this->values.set(5, value++);
			this->values.set(JOKER_POINT, value++);
			this->values.set(7, value++);
			this->players = vector<vector<Card>>(playerCount);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->dealer = INVALID_PLAYER;
			this->lastHand = Hand{};
			this->amounts.clear();
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
};

class Interaction
{
private:
	const vector<string> helpOptions = { "?", "/?", "-?", "h", "/h", "-h", "help", "/help", "--help" };
	HelpKey helpKey = 0;
	const vector<string> pokerTypeOptions = { "t", "/t", "-t", "pokerType", "/pokerType", "--pokerType" };
	const vector<string> pokerTypesC = { "斗地主", "斗地主拓展版", "四人斗地主", "锄大地", "三两一", "五瓜皮", "七鬼五二一", "七鬼五二三" };
	const vector<string> pokerTypesE = { "Landlords", "LandlordsX", "Landlords4P", "BigTwo", "ThreeTwoOne", "Wuguapi", "Qiguiwueryi", "Qiguiwuersan" };
	string pokerType = "扑克牌";
	const vector<string> playerCountOptions = { "p", "/p", "-p", "playerCount", "/playerCount", "--playerCount" };
	size_t playerCount = 0;
	const vector<string> sortingOptions = { "s", "/s", "-s", "sorting", "/sorting", "--sorting" };
	vector<Sorting> sortings{};
	PokerGame* pokerGame = nullptr;
	const vector<string> landlordStatements = { "Y", "yes", "1", "T", "true", "是", "叫", "叫地主", "叫牌", "抢", "抢地主", "抢牌" };
	const vector<string> againStatements = { "Again", "再来", "再来一局", "新开", "新开一局" };
	const vector<string> returnStatements = { "Return", "返回", "返回主面板", "返回主界面" };
	const vector<string> exitStatements = { "Exit", "Ctrl+C", "Ctrl + C", "退出", "退出程序" };

	/* Command line handling */
	const bool isEqual(const string& s1, const string& s2) const // Please use == directly if cases cannot be ignored
	{
		if (s1.length() == s2.length())
		{
			const size_t length = s1.length();
			for (size_t idx = 0; idx < length; ++idx)
			{
				char c1 = s1.at(idx), c2 = s2.at(idx);
				if ('A' <= c1 && c1 <= 'Z')
					c1 ^= 0x20;
				if ('A' <= c2 && c2 <= 'Z')
					c2 ^= 0x20;
				if (c1 != c2)
					return false;
			}
			return true;
		}
		else
			return false;
	}
	const bool isIn(const string& s, const vector<string>& strings) const
	{
		for (const string& str : strings)
			if (this->isEqual(s, str))
				return true;
		return false;
	}
	const void replaceAll(string& str, const string& oldSubString, const string& newSubString) const
	{
		size_t pos = 0;
		while ((pos = str.find(oldSubString, pos)) != string::npos)
		{
			str.replace(pos, oldSubString.length(), newSubString);
			pos += newSubString.length();
		}
		return;
	}
	const void optimizePokerType(string& _pokerType) const
	{
		if (!this->isIn(_pokerType, this->pokerTypesE))
		{
			_pokerType.erase(remove_if(_pokerType.begin(), _pokerType.end(), [](char ch) { return ' ' == ch || '\t' == ch || '\r' == ch || '\n' == ch; }), _pokerType.end());
			this->replaceAll(_pokerType, "D", "地");
			this->replaceAll(_pokerType, "7", "七");
			this->replaceAll(_pokerType, "5", "五");
			this->replaceAll(_pokerType, "3", "三");
			this->replaceAll(_pokerType, "2", "二");
			this->replaceAll(_pokerType, "1", "一");
			this->replaceAll(_pokerType, "三人斗地主", "斗地主");
			this->replaceAll(_pokerType, "欢乐斗地主", "斗地主");
			this->replaceAll(_pokerType, "竞技二打一扑克", "斗地主");
			this->replaceAll(_pokerType, "竞技二打一", "斗地主");
			this->replaceAll(_pokerType, "二打一", "斗地主");
			this->replaceAll(_pokerType, "大老二", "锄大地");
			this->replaceAll(_pokerType, "锄大弟", "锄大地");
		}
		return;
	}
	const string vector2string(const vector<string>& strings, const string& prefix, const string& separator, const string& suffix) const
	{
		string stringBuffer = prefix;
		const size_t length = strings.size();
		if (length >= 1)
		{
			stringBuffer += strings[0];
			for (size_t stringID = 1; stringID < length; ++stringID)
				stringBuffer += separator + strings[stringID];
		}
		stringBuffer += suffix;
		return stringBuffer;
	}
	const bool printHelp()
	{
		cout << "通用扑克牌实现与解杀程序。" << endl << endl << endl;
		switch (this->helpKey)
		{
		case 'T':
		case 't':
		{
			cout << "目前支持以下扑克牌类型：" << endl;
			const size_t length = this->pokerTypesC.size();
			for (size_t idx = 0; idx < length;)
				cout << "\t（" << ++idx << "）" << this->pokerTypesC[idx] << "；" << endl;
			cout << endl << endl << endl;
			return true;
		}
		case 'P':
		case 'p':
			cout << "用于指定玩家人数的参数目前仅对以下扑克牌类型生效：" << endl;
			cout << "\t（1）五瓜皮：最少 2 人，最多 10 人，默认 2 人；" << endl;
			cout << "\t（2）七鬼五二一：最少 2 人，最多 7 人，默认 2 人；" << endl;
			cout << "\t（3）七鬼五二三：最少 2 人，最多 7 人，默认 2 人。" << endl << endl;
			cout << "否则，该参数将会被自动忽略。" << endl << endl << endl << endl;
			return true;
		case 'S':
		case 's':
			cout << "目前支持以下排序显示方式：" << endl;
			cout << "\t（1）从大到小排序（默认）：FromBigToSmall (0)；" << endl;
			cout << "\t（2）从小到大排序：FromSmallToBig (1)；" << endl;
			cout << "\t（3）从多到少排序：FromManyToFew (2)；" << endl;
			cout << "\t（4）从少到多排序：FromFewToMany (3)。" << endl << endl << endl << endl;
			return true;
		default:
			cout << "参数（不区分顺序和大小写）：" << endl;
			cout << "\t" << this->vector2string(this->pokerTypeOptions, "[", "|", "]") << " [扑克牌类型]\t\t\t设置扑克牌类型" << endl;
			cout << "\t" << this->vector2string(this->playerCountOptions, "[", "|", "]") << " [玩家人数]\t\t设置玩家人数" << endl;
			cout << "\t" << this->vector2string(this->sortingOptions, "[", "|", "]") << " [排序显示方式]\t设置排序显示方式" << endl;
			cout << "\t" << this->vector2string(this->helpOptions, "[", "|", "]") << " 或 [其它参数] " << this->vector2string(this->helpOptions, "[", "|", "]") << "\t显示帮助" << endl << endl << endl;
			cout << "注意：" << endl;
			cout << "\t（1）键和值应当成对出现，即每一个表示键的参数后均应紧接着其对应的值（含帮助参数）；" << endl;
			cout << "\t（2）当同一键出现多次时，其值以该键最后一次以合法键值对的形式出现时的值为准；" << endl;
			cout << "\t（3）出现多个帮助参数时，以最后一次出现帮助参数时的上下文进行显示帮助。" << endl << endl << endl << endl;
			return 1 == this->helpKey;
		}
	}

	/* Interactive handling */
	const int clearScreen() const
	{
#if defined _WIN32 || defined _WIN64
		return system("cls");
#else
		return system("clear");
#endif
	}
	const void rfstdin() const
	{
		rewind(stdin);
		fflush(stdin);
		return;
	}
	const void getDescription(string& description) const
	{
		this->rfstdin();
		description.clear();
		char c = 0;
		while (cin.get(c) && c != '\n')
			description += c;
		return;
	}
	const size_t fetchPlayerCount(const size_t _lowerBound, const size_t upperBound) const // lowerBound must be not smaller than 2
	{
		const size_t lowerBound = _lowerBound >= 2 ? _lowerBound : 2;
		if (lowerBound <= this->playerCount && this->playerCount <= upperBound)
			return this->playerCount;
		else
		{
			this->clearScreen();
			cout << "已选定扑克牌类型：" << this->pokerType << endl;
			cout << "该扑克牌类型支持最少 " << lowerBound << " 人，最多 " << upperBound << " 人。" << endl;
			for (;;)
			{
				string playerCountString{};
				cout << "请输入玩家人数（输入“/”并按下回车键将使用默认值）：";
				this->getDescription(playerCountString);
				if ("/" == playerCountString)
					return lowerBound;
				else
				{
					const unsigned long int playerCountUL = strtoul(playerCountString.c_str(), NULL, 0);
					if (lowerBound <= playerCountUL && playerCountUL <= upperBound)
						return (size_t)playerCountUL;
				}
			}
		}
	}
	const bool fetchPokerType()
	{
		this->optimizePokerType(this->pokerType);
		if (!this->isIn(this->pokerType, this->pokerTypesC) && !this->isIn(this->pokerType, this->pokerTypesE))
			for (;;)
			{
				this->clearScreen();
				cout << "可选的扑克牌类型如下：" << endl;
				const size_t length = this->pokerTypesC.size();
				for (size_t idx = 0; idx < length; ++idx)
					cout << "\t" << (idx + 1) << " = " << this->pokerTypesC[idx] << endl;
				cout << "\t0 = 退出程序" << endl << endl << "请选择或输入一种扑克牌以继续：";
				string _pokerType{};
				this->getDescription(_pokerType);
				if (_pokerType.size() == 1)
				{
					const char choice = _pokerType.at(0) - '1';
					if (-1 == choice)
						return false;
					else if (0 <= choice && choice < static_cast<char>(this->pokerTypesC.size()))
					{
						this->pokerType = this->pokerTypesC[choice];
						break;
					}
				}
				else
				{
					this->optimizePokerType(_pokerType);
					if (this->isIn(_pokerType, this->pokerTypesC) || this->isIn(_pokerType, this->pokerTypesE))
					{
						this->pokerType = _pokerType;
						break;
					}
					else if ("退出程序" == _pokerType || this->isEqual("Exit", _pokerType))
						return false;
				}
			}
		if ("五瓜皮" == this->pokerType || this->isEqual("Wuguapi", this->pokerType))
			this->playerCount = fetchPlayerCount(2, 10);
		else if ("七鬼五二一" == this->pokerType || this->isEqual("Qiguiwueryi", this->pokerType) || "七鬼五二三" == this->pokerType || this->isEqual("Qiguiwuersan", this->pokerType))
			this->playerCount = fetchPlayerCount(2, 7);
		else
			this->playerCount = 0;
		return true;
	}
	const bool fetchBinaryChars(const string& filePath, vector<char>& binaryChars) const // return true if the parameter is a good file to be read
	{
		ifstream ifs(filePath, ios::binary | ios::ate);
		if (ifs.is_open())
		{
			streampos fileSize = ifs.tellg();
			ifs.seekg(0, std::ios::beg);
			binaryChars = vector<char>((size_t)fileSize);
			const bool flag = static_cast<bool>(ifs.read(binaryChars.data(), fileSize));
			ifs.close();
			return flag;
		}
		else
			return false;
	}

	/* Action confirmation */
	const bool ensureAction(const string& buffer, const string& actionDescriptioin) const
	{
		string repeatedBuffer{};
		cout << "您确定要" << actionDescriptioin << "吗？请再次输入以上指令以确认：";
		this->getDescription(repeatedBuffer);
		return buffer == repeatedBuffer;
	}
	const bool controlAction(const string& buffer, Action& action) const
	{
		if (this->isIn(buffer, this->againStatements))
		{
			if (this->ensureAction(buffer, "新开一局"))
			{
				action = Action::Again;
				return true;
			}
		}
		else if (this->isIn(buffer, this->returnStatements))
		{
			if (this->ensureAction(buffer, "返回主界面"))
			{
				action = Action::Return;
				return true;
			}
		}
		else if (this->isIn(buffer, this->exitStatements))
		{
			if (this->ensureAction(buffer, "退出程序"))
			{
				action = Action::Exit;
				return true;
			}
		}
		return false;
	}

	/* Procedures */
	const bool setLandlord(Action& action) const
	{
		Player player = INVALID_PLAYER;
		string buffer{};
		Count retryCount = 0;
		for (;;)
		{
			bool isRobbing = false;
			Count callerAndRobberCount = 0;
			for (Count count = 0; count < 3;)
			{
				this->pokerGame->getCurrentPlayer(player);
				this->clearScreen();
				this->pokerGame->display(this->sortings.empty() ? INVALID_PLAYER : player);
				cout << "请玩家 " << (player + 1) << " 选择是否" << (isRobbing ? "抢" : "叫") << "地主：";
				this->getDescription(buffer);
				if (this->controlAction(buffer, action))
					return false;
				else if (!buffer.empty())
					if (this->isIn(buffer, this->landlordStatements))
					{
						if (this->pokerGame->setLandlord(true))
						{
							isRobbing = true;
							++callerAndRobberCount;
							++count;
						}
					}
					else if (this->pokerGame->setLandlord(false))
						++count;
			}
			if (callerAndRobberCount >= 2)
				for (;;)
				{
					this->pokerGame->getCurrentPlayer(player);
					this->clearScreen();
					this->pokerGame->display(this->sortings.empty() ? INVALID_PLAYER : player);
					cout << "请玩家 " << (player + 1) << " 选择是否抢地主：";
					this->getDescription(buffer);
					if (this->controlAction(buffer, action))
						return false;
					else if (this->pokerGame->setLandlord(this->isIn(buffer, this->landlordStatements)))
						break;
				}
			else if (0 == callerAndRobberCount && ++retryCount < 3)
			{
				cout << "无人叫地主，即将重新发牌。" << endl;
				this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
				this->pokerGame->deal();
				continue;
			}
			break;
		}
		return true;
	}
	const bool setLandlord4P(Action& action) const
	{
		Player player = INVALID_PLAYER;
		string buffer{};
		Count retryCount = 0;
		for (;;)
		{
			Score currentHighestScore = Score::None;
			vector<string> scoreDescriptions{ "不叫", "3分", "2分", "1分" };
			for (Count count = 1; count <= 4 && currentHighestScore < Score::Three;)
			{
				this->pokerGame->getCurrentPlayer(player);
				this->clearScreen();
				this->pokerGame->display(this->sortings.empty() ? INVALID_PLAYER : player);
				cout << "请玩家 " << (player + 1) << " 选择（" << this->vector2string(scoreDescriptions, "", " | ", "") << "）：";
				this->getDescription(buffer);
				if (this->controlAction(buffer, action))
					return false;
				else if (!buffer.empty())
				{
					const char scoreChar = buffer.at(0) - '0';
					Score score = Score::None;
					switch (scoreChar)
					{
					case 1:
						score = Score::One;
						break;
					case 2:
						score = Score::Two;
						break;
					case 3:
						score = Score::Three;
						break;
					default:
						break;
					}
					if (this->pokerGame->setLandlord(score))
					{
						if (score > currentHighestScore)
						{
							for (Count removalCount = static_cast<Count>(score) - static_cast<Count>(currentHighestScore); removalCount > 0; --removalCount)
								scoreDescriptions.pop_back();
							currentHighestScore = score;
						}
						++count;
					}
				}
			}
			if (currentHighestScore >= Score::One || retryCount >= 2)
				break;
			else
			{
				++retryCount;
				cout << "无人叫地主，即将重新发牌。" << endl;
				this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
				this->pokerGame->deal();
			}
		}
		return true;
	}
	const void resetAll()
	{
		this->helpKey = 0;
		this->pokerType = "扑克牌";
		this->playerCount = 0;
		this->sortings = vector<Sorting>{};
		if (this->pokerGame != nullptr)
		{
			delete this->pokerGame;
			this->pokerGame = nullptr;
		}
		return;
	}
	const bool selectType(vector<Candidate>& candidates, Action& action) const
	{
		cout << "当前牌组存在二义性，在当前环境中，所有可能的牌型列举如下：" << endl;
		const size_t length = candidates.size();
		for (size_t idx = 0; idx < length; ++idx)
			cout << "\t" << (idx + 1) << " = " << candidates[idx].description << endl;
		for (;;)
		{
			cout << endl << "请选择一种牌型以继续（输入“/”并按下回车键将重新选择要出的牌）：";
			string buffer{};
			this->getDescription(buffer);
			if (this->controlAction(buffer, action) || "/" == buffer)
				return false;
			else
			{
				const unsigned long int choice = strtoul(buffer.c_str(), nullptr, 0) - 1;
				if (0 <= choice && choice < length)
				{
					candidates = vector<Candidate>{ candidates[choice] };
					return true;
				}
			}
		}
	}
	const bool start(Action& action) const
	{
		Player player = INVALID_PLAYER;
		string buffer{};
		this->pokerGame->getCurrentPlayer(player);
		for (;;)
		{
			this->clearScreen();
			this->pokerGame->display(this->sortings.empty() ? INVALID_PLAYER : player);
			cout << "请玩家 " << (player + 1) << " 开牌：";
			this->getDescription(buffer);
			if (this->controlAction(buffer, action))
				return false;
			else
			{
				vector<Candidate> candidates{};
				if (this->pokerGame->start(buffer, candidates))
					return true;
				else if (candidates.empty())
				{
					cout << "无法使用所选牌组开牌，请重新选择要出的牌。" << endl;
					this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
				}
				else if (this->selectType(candidates, action))
				{
					if (this->pokerGame->start(candidates[0].hand.cards, candidates))
						return true;
				}
				else if (action != Action::None)
					return false;
			}
		}
	}
	const bool play(Action& action) const
	{
		Player player = INVALID_PLAYER;
		string buffer{};
		this->pokerGame->getCurrentPlayer(player);
		while (player != INVALID_PLAYER)
		{
			for (;;)
			{
				this->clearScreen();
				this->pokerGame->display(this->sortings.empty() ? INVALID_PLAYER : player);
				cout << "请玩家 " << (player + 1) << " 出牌：";
				this->getDescription(buffer);
				if (this->controlAction(buffer, action))
					return false;
				else
				{
					vector<Candidate> candidates{};
					if (this->pokerGame->play(buffer, candidates))
						break;
					else if (candidates.empty())
					{
						cout << "无法使用所选牌组开牌，请重新选择要出的牌。" << endl;
						this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
					}
					else if (this->selectType(candidates, action))
					{
						if (this->pokerGame->play(candidates[0].hand.cards, candidates))
							break;
					}
					else if (action != Action::None)
						return false;
				}
			}
			this->pokerGame->getCurrentPlayer(player);
		}
		return true;
	}
	
public:
	Interaction()
	{

	}
	Interaction(const vector<string>& arguments)
	{
		if (!arguments.empty())
		{
			vector<size_t> invalidArgumentIndexes{};
			const size_t argumentCount = arguments.size() - 1;
			size_t argumentID = 0;
			for (; argumentID < argumentCount; ++argumentID)
				if (this->isIn(arguments[argumentID], this->helpOptions))
					this->helpKey = 1;
				else if (this->isIn(arguments[argumentID], this->pokerTypeOptions))
					if (this->isIn(arguments[++argumentID], this->helpOptions))
						this->helpKey = 't';
					else
					{
						string _pokerType = arguments[argumentID];
						this->optimizePokerType(_pokerType);
						if (this->isIn(_pokerType, this->pokerTypesC) || this->isIn(_pokerType, this->pokerTypesE))
							this->pokerType = _pokerType;
						else
							invalidArgumentIndexes.push_back(argumentID);
					}
				else if (this->isIn(arguments[argumentID], this->playerCountOptions))
					if (this->isIn(arguments[++argumentID], this->helpOptions))
						this->helpKey = 'p';
					else
					{
						const size_t _playerCount = (size_t)strtoul(arguments[argumentID].c_str(), NULL, 0);
						if (2 <= _playerCount && _playerCount <= 10)
							this->playerCount = _playerCount;
						else
							invalidArgumentIndexes.push_back(argumentID);
					}
				else if (this->isIn(arguments[argumentID], this->sortingOptions))
					if (this->isIn(arguments[++argumentID], this->helpOptions))
						this->helpKey = 's';
					else
						invalidArgumentIndexes.push_back(argumentID);
				else
					invalidArgumentIndexes.push_back(argumentID);
			if (argumentID == argumentCount)
				if (this->isIn(arguments[argumentID], this->helpOptions))
					this->helpKey = 1;
				else
					invalidArgumentIndexes.push_back(argumentID);
			if (!invalidArgumentIndexes.empty())
			{
				const size_t length = invalidArgumentIndexes.size();
				cout << "警告：以下 " << length << " 个参数无效。" << endl;
				for (size_t idx = 0; idx < length; ++idx)
					cout << "（" << (idx + 1) << "）参数 " << (invalidArgumentIndexes[idx] + 1) << " 无效——“" << arguments[invalidArgumentIndexes[idx]] << "”，其键值对（如有）已被自动跳过。" << endl;
				cout << endl;
				this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP * length));
			}
		}
	}
	const bool interact()
	{
		if (this->helpKey)
			return this->printHelp();
		else
			for (;;)
			{
				this->pokerGame = nullptr;
				while (nullptr == this->pokerGame)
				{
					if ("斗地主" == this->pokerType || this->isEqual("Landlords", this->pokerType)) // "三人斗地主"
						this->pokerGame = new Landlords;
					else if ("四人斗地主" == this->pokerType || this->isEqual("Landlords4P", this->pokerType)) // "四人斗地主"
						this->pokerGame = new Landlords4P;
					else if ("锄大地" == this->pokerType || this->isEqual("BigTwo", this->pokerType)) // "锄大地"
						this->pokerGame = new BigTwo;
					else if ("三两一" == this->pokerType || this->isEqual("ThreeTwoOne", this->pokerType)) // "三两一"
						this->pokerGame = new ThreeTwoOne;
					else if ("五瓜皮" == this->pokerType || this->isEqual("Wuguapi", this->pokerType)) // "五瓜皮"
						this->pokerGame = new Wuguapi;
					else if ("七鬼五二一" == this->pokerType || this->isEqual("Qiguiwueryi", this->pokerType)) // "七鬼五二一"
						this->pokerGame = new Qiguiwueryi;
					else if ("七鬼五二三" == this->pokerType || this->isEqual("Qiguiwuersan", this->pokerType)) // "七鬼五二三"
						this->pokerGame = new Qiguiwueryi;
					else if (!this->fetchPokerType())
						return true;
				}
				this->fetchPokerType();
				if (this->playerCount ? this->pokerGame->initialize(this->playerCount) : this->pokerGame->initialize())
				{
					/* Beginning */
					this->clearScreen();
					cout << "当前牌局（" << this->pokerType << "）已初始化，但暂未开局，请发牌或录入残局数据。" << endl << "请输入“/”并按下回车键开局，或录入残局库数据：";
					for (;;)
					{
						string buffer{};
						this->getDescription(buffer);
						if ("/" == buffer)
							if (this->pokerGame->deal())
								break;
							else
								cout << "开局失败！请再次尝试输入“/”并按下回车键开局，或录入残局库数据：";
						else
						{
							vector<char> binaryChars{};
							this->fetchBinaryChars(buffer, binaryChars);
							if (this->pokerGame->set(binaryChars))
								break;
							else
								cout << "录入失败！请输入“/”并按下回车键开局，或再次尝试录入残局库数据：";
						}
					}

					/* PokerGame::setLandlord(4P) */
					Action action = Action::None;
					if ("斗地主" == this->pokerType)
						this->setLandlord(action);
					else if ("四人斗地主" == this->pokerType)
						this->setLandlord4P(action);
					switch (action)
					{
					case Action::Return:
						this->resetAll();
					case Action::Again:
						continue;
					case Action::Exit:
						return true;
					default:
						break;
					}

					/* PokerGame::start */
					this->start(action);
					switch (action)
					{
					case Action::Return:
						this->resetAll();
					case Action::Again:
						continue;
					case Action::Exit:
						return true;
					default:
						break;
					}

					/* PokerGame::play */
					this->play(action);
					switch (action)
					{
					case Action::Return:
						this->resetAll();
					case Action::Again:
						continue;
					case Action::Exit:
						return true;
					default:
						break;
					}

					/* Ending */
					this->clearScreen();
					this->pokerGame->display();
					cout << "此局已终，请按 A 键回车再来一局，按 E 键回车退出程序，按其它键回车更改扑克牌游戏：";
					this->rfstdin();
					switch (getchar())
					{
					case 'A':
					case 'a':
						break;
					case 'E':
					case 'e':
						return true;
					default:
						this->resetAll();
					}
				}
				else
				{
					cout << "错误：初始化实例失败。" << endl << endl << endl << endl;
					this->resetAll();
					return false;
				}
			}
	}
};



int main(int argc, char* argv[])
{
	vector<string> arguments(argc - 1);
	for (int i = 0; i < argc - 1; ++i)
		arguments[i] = argv[i + 1];
	Interaction interaction = Interaction(arguments);
	return interaction.interact() ? EXIT_SUCCESS : EXIT_FAILURE;
}