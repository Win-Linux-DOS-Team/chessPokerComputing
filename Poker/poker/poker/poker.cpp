#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <functional>
#if !defined _WIN32 && !defined _WIN64 && !defined WIN32 && !defined WIN64
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
#define UNREFERENCED_PARAMETER(P) (void(P))
#endif
typedef unsigned char HelpKey;
typedef unsigned short Order;
typedef unsigned char Value;
typedef unsigned char Point; // Please remove the comments by searching the std::string "/* 0 <= " if the ``unsigned`` here is removed. 
typedef unsigned char Player; // Please remove the comments by searching the std::string "/* 0 <= " if the ``unsigned`` here is removed. 
typedef unsigned char Count;
typedef unsigned char Integral;
typedef long long int Amount;
typedef Player Ranking;
constexpr const long long int TIME_FOR_SLEEP = 3;
constexpr const char* const DEFAULT_STRING = "/";
constexpr const Point JOKER_POINT = 0;
constexpr const Player INVALID_PLAYER = (Player)(-1);


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
	Empty = 0x00, // 0b00000000
	
	Single = 0x10, // 0b00010000
	SingleStraight = 0x11, // 0b00010001
	SingleFlush = 0x12, // 0b00010010
	SingleFlushStraight = 0x13, // 0b00010011
	
	Pair = 0x20, // 0b00100000
	PairStraight = 0x21, // 0b00100001
	PairStraightWithSingle = 0x22, // 0b00100010
	PairJokers = 0x23, // 0b00100011
	
	Triple = 0x30, // 0b00110000
	TripleWithSingle = 0x31, // 0b00110001
	TripleWithPair = 0x32, // 0b00110010
	TripleWithPairSingle = 0x33, // 0b00110011
	TripleStraight = 0x34, // 0b00110100
	TripleStraightWithSingle = 0x35, // 0b00110101
	TripleStraightWithSingles = 0x36, // 0b00110110
	TripleStraightWithPairs = 0x37, // 0b00110111
	
	Quadruple = 0x40, // 0b01000000
	QuadrupleWithSingle = 0x41, // 0b01000001
	QuadrupleWithSingleSingle = 0x42, // 0b01000010
	QuadrupleWithPairPair = 0x43, // 0b01000011
	QuadrupleStraight = 0x44, // 0b01000100
	QuadrupleStraightWithSingle = 0x45, // 0b01000101
	QuadrupleJokers = 0x46, // 0b01000110
	
	Quintuple = 0x50, // 0b01010000
	Sextuple = 0x60, // 0b01100000
	Septuple = 0x70, // 0b01110000
	Octuple = 0x80, // 0b10000000
	
	Invalid = 0xFF // 0b11111111
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

enum class Score : unsigned char
{
	None = 0, 
	One = 1, 
	Two = 2, 
	Three = 3
};

enum class Action : unsigned char
{
	None = 000, 
	AgainCancelled = 001, 
	ReturnCancelled = 002, 
	ExitCancelled = 003, 
	AgainConfirmed = 011, 
	ReturnConfirmed = 012, 
	ExitConfirmed = 013
};


struct Card
{
	Point point = JOKER_POINT; // JOKER_POINT (0) is for the Jokers, the Cover Card, and the default value. 
	Suit suit = Suit::Cover;
	
	Card() : point(JOKER_POINT), suit(Suit::Cover) {}
	Card(const Point point) : point(/* 0 <= point && */point <= 13 ? point : JOKER_POINT), suit(Suit::Cover) {}
	Card(const Point point, const Suit suit) : point(/* 0 <= point && */point <= 13 ? point : JOKER_POINT), suit(((1 <= point && point <= 13 && (suit <= Suit::Spade || Suit::Cover == suit)) || (JOKER_POINT == point && suit >= Suit::Black)) ? suit : Suit::Cover) {}
	friend bool operator==(const Card& a, const Card& b)
	{
		return a.point == b.point && a.suit == b.suit;
	}
	operator const std::string() const // this->point + this->suit = 先花色后点数（中文习惯） | this->point + Suit::Cover = 仅播报点数 | JOKER_POINT (0) + this->suit = 仅播报花色 | JOKER_POINT (0) + Suit::Cover = 广告牌
	{
		std::string stringBuffer{};
		switch (this->suit)
		{
		case Suit::Diamond:
			stringBuffer = "方块";
			break;
		case Suit::Club:
			stringBuffer = "梅花";
			break;
		case Suit::Heart:
			stringBuffer = "红桃";
			break;
		case Suit::Spade:
			stringBuffer = "黑桃";
			break;
		case Suit::Black:
			if (JOKER_POINT == this->point)
				return "小王";
			else
			{
				stringBuffer = "黑色";
				break;
			}
		case Suit::Red:
			if (JOKER_POINT == this->point)
				return "大王";
			else
			{
				stringBuffer = "红色";
				break;
			}
		case Suit::Cover:
		default:
			if (JOKER_POINT == this->point)
				return "封面牌";
			else
				break;
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
		default:
			break;
		}
		return stringBuffer;
	}
};

struct Hand
{
	Player player = INVALID_PLAYER;
	std::vector<Card> cards{};
	Type type = Type::Invalid;
	
	Hand() : player(INVALID_PLAYER), cards(std::vector<Card>{}), type(Type::Invalid) {}
	Hand(const Player player) = delete;
	Hand(const Player player, const std::vector<Card>& cards) : player(player), cards(cards), type(Type::Invalid) {}
	Hand(const Player player, const std::vector<Card>& cards, const Type type) = delete;
	friend bool operator==(const Hand& a, const Hand& b)
	{
		return a.player == b.player && a.cards == b.cards && a.type == b.type;
	}
	operator bool() const
	{
		return this->player != INVALID_PLAYER || this->type != Type::Invalid;
	}
};

struct Candidate
{
	Hand hand{};
	std::string description{};
	
	Candidate(const Hand& t, const std::string& s) : hand(t), description(s) {}
};


class Values
{
private:
	Value values[14] = { 0 };

public:
	Values()
	{
		
	}
	bool set(const Point point, const Value value)
	{
		if (/* 0 <= point && */point <= 13 && 1 <= value && value <= 14)
		{
			this->values[point] = value;
			return true;
		}
		else
			return false;
	}
#if (!defined _STL_WARNING_LEVEL || _STL_WARNING_LEVEL < 3)
	bool get(const Point point, Value& value) const
	{
		if (/* 0 <= point && */point <= 13)
		{
			value = this->values[point];
			return true;
		}
		else
			return false;
	}
#endif
	Value operator[](const Point point) const
	{
		return /* 0 <= point && */point <= 13 ? this->values[point] : (Value)0;
	}
};

class Poker
{
public:
	static const size_t MinimumPlayerCount = 2, MaximumPlayerCount = 10, CardCountPerPlayer = 0;
	
protected:
	std::mt19937 seed = std::mt19937(std::random_device{}());
	std::string name = "扑克牌";
	Values values{};
	std::vector<std::vector<Card>> players{};
	std::vector<Card> deck{};
	std::vector<std::vector<Hand>> records{};
	Player currentPlayer = INVALID_PLAYER, dealer = INVALID_PLAYER;
	Hand lastHand{};
	std::vector<Amount> amounts{};
	Status status = Status::Ready;
	
private:
	virtual void add52CardsToDeck(std::vector<Card>& _deck) const final
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
	virtual void add54CardsToDeck(std::vector<Card>& _deck) const final
	{
		this->add52CardsToDeck(_deck);
		_deck.push_back(Card{ JOKER_POINT, Suit::Black });
		_deck.push_back(Card{ JOKER_POINT, Suit::Red });
		return;
	}
	
protected:
	/* Poker::initialize */
	virtual bool checkPlayerCount(const size_t playerCount) const
	{
		return Poker::MinimumPlayerCount <= playerCount && playerCount <= Poker::MaximumPlayerCount;
	}
	
	/* Poker::deal */
	virtual void add52CardsToDeck() final
	{
		this->add52CardsToDeck(this->deck);
		return;
	}
	virtual void add54CardsToDeck() final
	{
		this->add54CardsToDeck(this->deck);
		return;
	}
	virtual bool sortCards(std::vector<Card>& cards, const Order order) const final
	{
		Order runningOrder = order;
		bool pointFlag = false, valueFlag = false, suitFlag = false, pointCountFlag = false, unionCountFlag = false, valueCountFlag = false, suitCountFlag = false;
		std::vector<std::function<int(const Card, const Card)>> lambdas{};
		Count pointCounts[14] = { 0 }, unionCounts[14][4] = { { 0 } }, valueCounts[15] = { 0 }, suitCounts[7] = { 0 };
		while (runningOrder) // From right to left, each 4 bits represent a single-level order. 
		{
			switch (runningOrder & 0xF/* 0b1111 */)
			{
			case 0x0: // 'P' (0b01010000) -> 0b0000 (Fetch the 1st, 2nd, 4th, and 6th bits)
				if (pointFlag)
					return false;
				else
				{
					pointFlag = true;
					lambdas.emplace_back([](const Card a, const Card b) { const Point pointA = a.point, pointB = b.point; return pointA > pointB ? -1 : pointA < pointB; });
					break;
				}
			case 0x2: // 'V' (0b01010110) -> 0b0010 (Fetch the 1st, 2nd, 4th, and 6th bits)
				if (valueFlag)
					return false;
				else
				{
					valueFlag = true;
					lambdas.emplace_back([this](const Card a, const Card b) { const Value valueA = this->values[a.point], valueB = this->values[b.point]; return valueA > valueB ? -1 : valueA < valueB; });
					break;
				}
			case 0x3: // 'S' (0b01010011) -> 0b0011 (Fetch the 1st, 2nd, 4th, and 6th bits)
				if (suitFlag)
					return false;
				else
				{
					suitFlag = true;
					lambdas.emplace_back([](const Card a, const Card b) { const Suit suitA = a.suit, suitB = b.suit; return suitA > suitB ? -1 : suitA < suitB; });
					break;
				}
			case 0x4: // 'H' (0b01001000) = 'P' (0b01010000) - 0b1000 -> 0b0100 (Fetch the 1st, 2nd, 4th, and 6th bits)
				if (pointCountFlag)
					return false;
				else
				{
					pointCountFlag = true;
					lambdas.emplace_back([&pointCounts](const Card a, const Card b) { const Count countA = pointCounts[a.point], countB = pointCounts[b.point]; return countA > countB ? -1 : countA < countB; });
					break;
				}
			case 0x5: // 'M' (0b01001101) = 'U' (0b01010101) - 0b1000 -> 0b0101 (Fetch the 1st, 2nd, 4th, and 6th bits)
				if (unionCountFlag)
					return false;
				else
				{
					unionCountFlag = true;
					lambdas.emplace_back([&unionCounts](const Card a, const Card b) { const Count countA = unionCounts[a.point][static_cast<unsigned char>(a.suit) & 0x3/* 0b11 */], countB = unionCounts[b.point][static_cast<unsigned char>(b.suit) & 0x3/* 0b11 */]; return countA > countB ? -1 : countA < countB; });
					break;
				}
			case 0x6: // 'N' (0b01001110) = 'V' (0b01010110) - 0b1000 -> 0b0110 (Fetch the 1st, 2nd, 4th, and 6th bits)
				if (valueCountFlag)
					return false;
				else
				{
					valueCountFlag = true;
					lambdas.emplace_back([&valueCounts,this](const Card a, const Card b) { const Count countA = valueCounts[this->values[a.point]], countB = valueCounts[this->values[b.point]]; return countA > countB ? -1 : countA < countB; });
					break;
				}
			case 0x7: // 'K' (0b01001011) = 'S' (0b01010011) - 0b1000 -> 0b0111 (Fetch the 1st, 2nd, 4th, and 6th bits)
				if (suitCountFlag)
					return false;
				else
				{
					suitCountFlag = true;
					lambdas.emplace_back([&suitCounts](const Card a, const Card b) { const Count countA = suitCounts[static_cast<unsigned char>(a.suit)], countB = suitCounts[static_cast<unsigned char>(b.suit)]; return countA > countB ? -1 : countA < countB; });
					break;
				}
			case 0x8: // 'p' (0b01110000) -> 0b1000 (Fetch the 1st, 2nd, 4th, and 6th bits)
				if (pointFlag)
					return false;
				else
				{
					pointFlag = true;
					lambdas.emplace_back([](const Card a, const Card b) { const Point pointA = a.point, pointB = b.point; return pointA < pointB ? -1 : pointA > pointB; });
					break;
				}
			case 0xA: // 'v' (0b01110110) -> 0b1010 (Fetch the 1st, 2nd, 4th, and 6th bits)
				if (valueFlag)
					return false;
				else
				{
					valueFlag = true;
					lambdas.emplace_back([this](const Card a, const Card b) { const Value valueA = this->values[a.point], valueB = this->values[b.point]; return valueA < valueB ? -1 : valueA > valueB; });
					break;
				}
			case 0xB: // 's' (0b01110011) -> 0b1011 (Fetch the 1st, 2nd, 4th, and 6th bits)
				if (suitFlag)
					return false;
				else
				{
					suitFlag = true;
					lambdas.emplace_back([](const Card a, const Card b) { const Suit suitA = a.suit, suitB = b.suit; return suitA < suitB ? -1 : suitA > suitB; });
					break;
				}
			case 0xC: // 'h' (0b01101000) = 'p' (0b01110000) - 0b1000 -> 0b1100 (Fetch the 1st, 2nd, 4th, and 6th bits)
				if (pointCountFlag)
					return false;
				else
				{
					pointCountFlag = true;
					lambdas.emplace_back([&pointCounts](const Card a, const Card b) { const Count countA = pointCounts[a.point], countB = pointCounts[b.point]; return countA < countB ? -1 : countA > countB; });
					break;
				}
			case 0xD: // 'm' (0b01101101) = 'u' (0b01110101) - 0b1000 -> 0b1101 (Fetch the 1st, 2nd, 4th, and 6th bits)
				if (unionCountFlag)
					return false;
				else
				{
					unionCountFlag = true;
					lambdas.emplace_back([&unionCounts](const Card a, const Card b) { const Count countA = unionCounts[a.point][static_cast<unsigned char>(a.suit) & 0x3/* 0b11 */], countB = unionCounts[b.point][static_cast<unsigned char>(b.suit) & 0x3/* 0b11 */]; return countA < countB ? -1 : countA > countB; });
					break;
				}
			case 0xE: // 'n' (0b01101110) = 'v' (0b01110110) - 0b1000 -> 0b1110 (Fetch the 1st, 2nd, 4th, and 6th bits)
				if (valueCountFlag)
					return false;
				else
				{
					valueCountFlag = true;
					lambdas.emplace_back([&valueCounts, this](const Card a, const Card b) { const Count countA = valueCounts[this->values[a.point]], countB = valueCounts[this->values[b.point]]; return countA < countB ? -1 : countA > countB; });
					break;
				}
			case 0xF: // 'k' (0b01101011) = 's' (0b01110011) - 0b1000 -> 0b1111 (Fetch the 1st, 2nd, 4th, and 6th bits)
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
			runningOrder >>= 4;
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
					++unionCounts[card.point][static_cast<unsigned char>(card.suit) & 0x3/* 0b11 */];
			if (valueCountFlag)
				for (const Card& card : cards)
					++valueCounts[this->values[card.point]];
			if (suitCountFlag)
				for (const Card& card : cards)
					++suitCounts[static_cast<unsigned char>(card.suit)];
			sort(cards.begin(), cards.end(), [&lambdas](const Card a, const Card b) { for (const std::function<int(const Card, const Card)>& lambda : lambdas) { const int result = lambda(a, b); if (result) return result > 0; } return false; });
		}
		return true;
	}
	virtual bool sortCards(std::vector<Card>& cards) const final
	{
		return this->sortCards(cards, 0xbA/* 0b10111010 */);
	}
	
	/* Poker::setLandlord */
	virtual bool nextPlayer()
	{
		const size_t playerCount = this->players.size();
		if (/* 0 <= this->currentPlayer && */this->currentPlayer < playerCount)
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
	
	/* Poker::start */
	virtual bool description2cards(const std::string& description, std::vector<Card>& cards) const final
	{
		if (DEFAULT_STRING == description || "\\" == description || "-" == description || "--" == description || "要不起" == description || "不出" == description || "不打" == description)
		{
			cards.clear();
			return true;
		}
		else if (/* 0 <= this->currentPlayer && */this->currentPlayer < this->players.size() && !this->players[this->currentPlayer].empty())
		{
			std::vector<Card> exactCards{};
			std::vector<Point> fuzzyPoints{};
			std::vector<Suit> fuzzySuits{};
			const size_t descriptionLength = description.length();
			bool waitingForAPoint = false;
			Suit suit = Suit::Diamond;
			for (size_t idx = 0; idx < descriptionLength; ++idx)
				switch (description.at(idx))
				{
				case 'A':
				case 'a':
					if (waitingForAPoint)
					{
						exactCards.push_back(Card{ 1, suit });
						waitingForAPoint = false;
					}
					else
						fuzzyPoints.push_back(1);
					break;
				case '1':
					if (idx + 1 < descriptionLength && '0' == description.at(idx + 1))
					{
						if (waitingForAPoint)
						{
							exactCards.push_back(Card{ 10, suit });
							waitingForAPoint = false;
						}
						else
							fuzzyPoints.push_back(10);
						++idx;
					}
					else if (waitingForAPoint)
					{
						exactCards.push_back(Card{ 1, suit });
						waitingForAPoint = false;
					}
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
					{
						exactCards.push_back(Card{ (Point)(description.at(idx) - '0'), suit });
						waitingForAPoint = false;
					}
					else
						fuzzyPoints.push_back(static_cast<Player>(description.at(idx) - '0'));
					break;
				case 'T':
				case 't':
					if (waitingForAPoint)
					{
						exactCards.push_back(Card{ 10, suit });
						waitingForAPoint = false;
					}
					else
						fuzzyPoints.push_back(10);
					break;
				case 'J':
				case 'j':
					if (waitingForAPoint)
					{
						exactCards.push_back(Card{ 11, suit });
						waitingForAPoint = false;
					}
					else
						fuzzyPoints.push_back(11);
					break;
				case 'Q':
				case 'q':
					if (waitingForAPoint)
					{
						exactCards.push_back(Card{ 12, suit });
						waitingForAPoint = false;
					}
					else
						fuzzyPoints.push_back(12);
					break;
				case 'K':
				case 'k':
					if (waitingForAPoint)
					{
						exactCards.push_back(Card{ 13, suit });
						waitingForAPoint = false;
					}
					else
						fuzzyPoints.push_back(13);
					break;
				case 'R': // 'B' can for "Black" and "Big" so not good for use
				case 'r': // 'b' can for "black" and "big" so not good for use
					if (waitingForAPoint)
					{
						fuzzySuits.push_back(suit);
						waitingForAPoint = false;
					}
					else
						exactCards.push_back(Card{ JOKER_POINT, Suit::Red });
					break;
				case 'L': // 'B' can for "Black" and "Big" so not good for use
				case 'l': // 'b' can for "black" and "big" so not good for use
					if (waitingForAPoint)
					{
						fuzzySuits.push_back(suit);
						waitingForAPoint = false;
					}
					else
						exactCards.push_back(Card{ JOKER_POINT, Suit::Black });
					break;
				case 'D':
				case 'd':
					if (waitingForAPoint)
					{
						fuzzySuits.push_back(suit);
						waitingForAPoint = false;
					}
					suit = Suit::Diamond;
					break;
				case 'C':
				case 'c':
					if (waitingForAPoint)
					{
						fuzzySuits.push_back(suit);
						waitingForAPoint = false;
					}
					suit = Suit::Club;
					break;
				case 'H':
				case 'h':
					if (waitingForAPoint)
					{
						fuzzySuits.push_back(suit);
						waitingForAPoint = false;
					}
					suit = Suit::Heart;
					break;
				case 'S':
				case 's':
					if (waitingForAPoint)
					{
						fuzzySuits.push_back(suit);
						waitingForAPoint = false;
					}
					suit = Suit::Spade;
					break;
				default:
				{
					if (waitingForAPoint)
					{
						fuzzySuits.push_back(suit);
						waitingForAPoint = false;
					}
					const std::string str = description.substr(idx, 4);
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
			if (waitingForAPoint)
				fuzzySuits.push_back(suit);
			std::vector<size_t> selected{};
			const size_t length = this->players[this->currentPlayer].size();
			for (const Card& exactCard : exactCards) // select the rightmost one
			{
				size_t position = length;
				const Value value = this->values[exactCard.point];
				for (size_t idx = length - 1; idx > 0; --idx)
					if (exactCard.point == this->players[this->currentPlayer][idx].point)
					{
						if (exactCard.suit == this->players[this->currentPlayer][idx].suit)
						{
							if (find(selected.begin(), selected.end(), idx) == selected.end())
							{
								position = idx;
								break;
							}
						}
						else if (exactCard.suit < this->players[this->currentPlayer][idx].suit)
							break;
					}
					else if(value < this->values[this->players[this->currentPlayer][idx].point])
						break;
				if (position < length)
					selected.emplace_back(std::move(position));
				else if (exactCard == this->players[this->currentPlayer][0]) // avoid (size_t)(-1)
					selected.emplace_back(static_cast<size_t>(0));
				else
					return false;
			}
			for (const Point& fuzzyPoint : fuzzyPoints) // search for the smallest suit that is not selected for each point to select
			{
				size_t position = length;
				const Value value = this->values[fuzzyPoint];
				for (size_t idx = length - 1; idx > 0; --idx)
					if (fuzzyPoint == this->players[this->currentPlayer][idx].point)
					{
						if (find(selected.begin(), selected.end(), idx) == selected.end())
						{
							position = idx;
							break;
						}
					}
					else if (value < this->values[this->players[this->currentPlayer][idx].point])
						break;
				if (position < length)
					selected.emplace_back(std::move(position));
				else if (fuzzyPoint == this->players[this->currentPlayer][0].point) // avoid (size_t)(-1)
					selected.emplace_back(static_cast<size_t>(0));
				else
					return false;
			}
			for (const Suit& fuzzySuit : fuzzySuits) // search for the point with the smallest value that is not selected for each suit to select
			{
				size_t position = length;
				for (size_t idx = length - 1; idx > 0; --idx)
					if (this->players[this->currentPlayer][idx].suit == fuzzySuit && find(selected.begin(), selected.end(), idx) == selected.end())
					{
						position = idx;
						break;
					}
				if (position < length)
					selected.emplace_back(std::move(position));
				else if (fuzzySuit == this->players[this->currentPlayer][0].suit) // avoid (size_t)(-1)
					selected.emplace_back(static_cast<size_t>(0));
				else
					return false;
			}
			cards.clear();
			for (const size_t& position : selected)
				cards.push_back(this->players[this->currentPlayer][position]);
			return true;
		}
		else
			return false;
	}
	virtual bool checkStarting(const std::vector<Card>& cards) const
	{
		return !cards.empty() && (this->records[0].back().cards.size() == 1 && find(cards.begin(), cards.end(), this->records[0].back().cards[0]) != cards.end());
	}
	virtual std::string point2description(const Point point) const final
	{
		switch (point)
		{
		case 1:
			return "A";
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			return std::to_string(point);
		case 11:
			return "J";
		case 12:
			return "Q";
		case 13:
			return "K";
		default:
			return "";
		}
	}
	virtual bool isRealHand(const Hand& hand) const = 0;
	virtual bool coverLastHand(const Hand& currentHand) const = 0;
	virtual bool judgeStraight(std::vector<Card>& cards, const Count repeatedCount, const Point pointNotAllowedToConnectK, const bool applySorting) const final // This function can only be used when every point is valid with the same count. 
	{
		const size_t cardCount = cards.size();
		if (1 <= repeatedCount && repeatedCount <= 4 && cardCount >= repeatedCount && cardCount % repeatedCount == 0 && 1 <= pointNotAllowedToConnectK && pointNotAllowedToConnectK <= 12)
		{
			std::vector<Card> sortedCards(cards);
			sort(sortedCards.begin(), sortedCards.end(), [](const Card a, const Card b) { return a.point > b.point || (a.point == b.point && a.suit > b.suit); });
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
					{
						if (sortedCards[idx += repeatedCount].point < pointNotAllowedToConnectK && 13 == sortedCards[0].point && 1 == sortedCards[indexToLastPoint].point)
						{
							const size_t indexToRotation = idx;
							for (; idx < indexToLastPoint; ++idx) // Straights from a point in [1, ``pointNotAllowedToConnectK`` - 1] to a point in [``pointNotAllowedToConnectK`` + 1, 13] and from a point in [1, ``pointNotAllowedToConnectK`` - 2] to a point in [``pointNotAllowedToConnectK``, 13]
								if (sortedCards[idx + 1].point + 1 != sortedCards[idx].point)
									return false;
							if (applySorting)
							{
								rotate(sortedCards.begin(), sortedCards.begin() + static_cast<ptrdiff_t>(indexToRotation), sortedCards.end());
								cards = sortedCards;
							}
							return true;
						}
						else
							return false;
					}
				if (applySorting)
					cards = sortedCards;
				return true;
			}
		}
		else
			return false;
	}
	virtual Count judgeStraight(std::vector<Card>& cards, const Point pointNotAllowedToConnectK, const bool applySorting) const final // This function can be used at any time. 
	{
		if (!cards.empty() && 1 <= pointNotAllowedToConnectK && pointNotAllowedToConnectK <= 12)
		{
			/* Packing */
			std::vector<Card> sortedCards(cards);
			sort(sortedCards.begin(), sortedCards.end(), [](const Card& a, const Card& b) { return a.point > b.point || (a.point == b.point && a.suit > b.suit); });
			Point lastPoint = JOKER_POINT;
			std::vector<std::vector<Card>> units{};
			for (const Card& card : sortedCards)
				if (JOKER_POINT == card.point)
					return 0;
				else if (card.point == lastPoint)
					units.back().push_back(card);
				else if (this->values[card.point])
				{
					units.push_back(std::vector<Card>{ card });
					lastPoint = card.point;
				}
				else
					return 0;
			
			/* Count computing */
			const size_t indexToLastBlock = units.size() - 1;
			const Count repeatedCount = static_cast<Count>(units[0].size());
			for (size_t idx = 1; idx <= indexToLastBlock; ++idx)
				if (units[idx].size() != repeatedCount)
					return 0;
			
			/* Straight judgement */
			if (12 == indexToLastBlock) // Straights from the largest to the smallest
				return repeatedCount; // No need to rotate
			for (size_t idx = 0; idx < indexToLastBlock; ++idx) // Straights like K ... 2
				if (units[idx + 1][0].point + 1 != units[idx][0].point)
				{
					if (units[++idx][0].point < pointNotAllowedToConnectK && 13 == units[0][0].point && 1 == units.back()[0].point)
					{
						const size_t indexToRotation = idx;
						for (; idx < indexToLastBlock; ++idx) // Straights from a point in [1, ``pointNotAllowedToConnectK`` - 1] to a point in [``pointNotAllowedToConnectK`` + 1, 13] and from a point in [1, ``pointNotAllowedToConnectK`` - 2] to a point in [``pointNotAllowedToConnectK``, 13]
							if (units[idx + 1][0].point + 1 != units[idx][0].point)
								return 0;
						if (applySorting)
							rotate(units.begin(), units.begin() + static_cast<ptrdiff_t>(indexToRotation), units.end());
						break;
					}
					else
						return 0;
				}
			if (applySorting)
			{
				size_t idx = 0;
				for (const std::vector<Card>& item : units)
					for (const Card& card : item)
						cards[idx++] = card;
			}
			return repeatedCount;
		}
		else
			return 0;
	}
	virtual bool processHand(Hand& hand, std::vector<Candidate>& candidates) const = 0;
	virtual bool removeCards(const std::vector<Card>& smallerCards, std::vector<Card>& largerCards) const final // The vector ``largerCards`` must have been sorted according to the default multi-level order. 
	{
		std::vector<Card> sortedCards(smallerCards);
		this->sortCards(sortedCards);
		const size_t smallerLength = sortedCards.size(), largerLength = largerCards.size();
		if (smallerLength > largerLength || 0 == largerLength)
			return false;
		else if (smallerLength >= 1)
		{
			std::vector<size_t> selected{};
			for (size_t smallerIndex = 0, largerIndex = 0; smallerIndex < smallerLength && largerIndex < largerLength; ++largerIndex)
				if (sortedCards[smallerIndex] == largerCards[largerIndex])
				{
					selected.push_back(largerIndex);
					++smallerIndex;
				}
			if (selected.size() == smallerLength)
			{
				for (size_t idx = smallerLength - 1; idx > 0; --idx)
					largerCards.erase(largerCards.begin() + static_cast<ptrdiff_t>(selected[idx]));
				largerCards.erase(largerCards.begin() + static_cast<ptrdiff_t>(selected[0]));
				return true;
			}
			else
				return false;
		}
		else
			return true;
	}
	virtual bool processBasis(const Hand& hand) { UNREFERENCED_PARAMETER(hand); return false; }
	virtual bool isOver() const
	{
		if (this->status >= Status::Started)
			for (const std::vector<Card>& cards : this->players)
				if (cards.empty())
					return true;
		return false;
	}
	virtual bool computeAmounts(const unsigned char multiplication1Opening7, const unsigned int basis12Calling4Robbing4Real4Empty4Spring4) { UNREFERENCED_PARAMETER(multiplication1Opening7); UNREFERENCED_PARAMETER(basis12Calling4Robbing4Real4Empty4Spring4); return false; }
	virtual bool computeAmounts() { return true; }
	virtual bool isAbsolutelyLargest(const Hand& hand) const = 0;
		
	/* Poker::display */
	virtual std::string getBasisString() const { return ""; }
	virtual std::string cards2string(const std::vector<Card>& cards, const std::string& prefix, const std::string& separator, const std::string& suffix, const std::string& returnIfEmpty) const final
	{
		if (cards.empty())
			return returnIfEmpty;
		else
		{
			std::string stringBuffer = prefix + (std::string)cards[0];
			const size_t length = cards.size();
			for (size_t cardID = 1; cardID < length; ++cardID)
				stringBuffer += separator + (std::string)cards[cardID];
			stringBuffer += suffix;
			return stringBuffer;
		}
	}
	virtual std::string getPreRoundString() const = 0;
	virtual std::string getAmountString() const
	{
		const size_t playerCount = this->players.size();
		if (Status::Over == this->status && playerCount == this->amounts.size() && this->checkPlayerCount(playerCount))
		{
			std::string amountString = "/* 结算信息 */\n";
			for (Player player = 0; player < playerCount; ++player)
				amountString += "玩家 " + std::to_string(player + 1) + "：" + std::to_string(this->amounts[player]) + "\n";
			return amountString;
		}
		else
			return "结算信息异常，请各位玩家自行计算结算信息。\n";
	}
	virtual bool display(const std::vector<Player>& selectedPlayers, const std::string& dealerRemark, const std::string& deckDescription) const final
	{
		switch (this->status)
		{
		case Status::Ready:
			std::cout << "牌局未初始化，请先初始化牌局。" << std::endl << std::endl;
			return true;
		case Status::Initialized:
			std::cout << "当前牌局（" << this->name << "）已初始化，但暂未开局，请发牌或录入残局数据。" << std::endl << std::endl;
			return true;
		case Status::Dealt:
		case Status::Assigned:
		case Status::Started:
		case Status::Over:
		{
			/* Beginning */
			bool flag = true;
			std::cout << "扑克游戏：" << this->name << "（";
			switch (this->status)
			{
			case Status::Dealt:
				std::cout << "已发牌";
				break;
			case Status::Assigned:
				std::cout << "等待开牌";
				break;
			case Status::Started:
				std::cout << "正在游戏";
				break;
			case Status::Over:
				std::cout << "已结束";
				break;
			case Status::Ready:
			case Status::Initialized:
			default:
				std::cout << "请检查进程内存";
				break;
			}
			std::cout << "）" << std::endl << this->getBasisString() << std::endl;

			/* Players */
			std::cout << "/* 玩家区域 */" << std::endl;
			const size_t playerCount = this->players.size();
			if (Status::Over == this->status)
				for (Player player = 0; player < playerCount; ++player)
					std::cout << "玩家 " << (player + 1) << (this->dealer == player ? "（" + dealerRemark + "）剩余 " : " 剩余 ") << this->players[player].size() << " 张扑克牌：" << this->cards2string(this->players[player], "\n", " | ", "", "（空）") << std::endl << std::endl;
			else
				for (Player player = 0; player < playerCount; ++player)
					if (find(selectedPlayers.begin(), selectedPlayers.end(), player) == selectedPlayers.end())
						std::cout << "玩家 " << (player + 1) << (this->dealer == player ? "（" + dealerRemark + "）剩余 " : " 剩余 ") << this->players[player].size() << " 张扑克牌：（不可见）" << std::endl << std::endl;
					else
						std::cout << "玩家 " << (player + 1) << (this->dealer == player ? "（" + dealerRemark + "）剩余 " : " 剩余 ") << this->players[player].size() << " 张扑克牌：" << this->cards2string(this->players[player], "\n", " | ", "", "（空）") << std::endl << std::endl;
			std::cout << deckDescription;

			/* Records */
			if (!this->records.empty())
			{
				std::cout << "/* 出牌记录 */" << std::endl;
				std::cout << "预备回合：" << this->getPreRoundString() << std::endl;
				const size_t roundCount = this->records.size();
				for (size_t round = 1; round < roundCount; ++round)
				{
					std::cout << "第 " << round << " 回合：";
					if (this->records[round].empty())
						std::cout << "（无）" << std::endl;
					else
					{
						char buffer[3] = { 0 };
						snprintf(buffer, 3, "%02X", static_cast<unsigned char>(this->records[round][0].type));
						std::cout << "{ 玩家 " << (this->records[round][0].player + 1) << ", " << this->cards2string(this->records[round][0].cards, "", " + ", "", "要不起") << ", 0x" << buffer << " }";
						const size_t handCount = this->records[round].size();
						for (size_t handID = 1; handID < handCount; ++handID)
						{
							snprintf(buffer, 3, "%02X", static_cast<unsigned char>(this->records[round][handID].type));
							std::cout << " -> { 玩家 " << (this->records[round][handID].player + 1) << ", " << this->cards2string(this->records[round][handID].cards, "", " + ", "", "要不起") << ", 0x" << buffer << " }";
						}
						std::cout << std::endl;
					}
				}
				std::cout << std::endl;

				/* Amounts */
				if (Status::Over == this->status)
					std::cout << this->getAmountString() << std::endl;
			}
			return flag;
		}
		default:
			std::cout << "当前牌局状态未知，无法显示牌局状况。" << std::endl << std::endl;
			return false;
		}
	}
	
public:
	Poker() // name and status = Status::Ready
	{
		
	}
	virtual ~Poker()
	{
		
	}
	virtual bool initialize() = 0; // values, players (= std::vector<std::vector<Card>>(n)), deck (clear), records (clear), currentPlayer (reset), dealer (reset), lastHand (reset), amounts (clear), and status = Status::Initialized
	virtual bool initialize(const size_t playerCount) { return this->checkPlayerCount(playerCount) && this->initialize(); } // values, players (= std::vector<std::vector<Card>>(n)), deck (clear), records (clear), currentPlayer (reset), dealer (reset), lastHand (reset), amounts (clear), and status = Status::Initialized
	virtual bool deal() = 0; // players, deck, records (clear) -> records[0], currentPlayer, dealer, lastHand (reset), amounts (clear) | amounts = std::vector<Amount>{ 0 }, and status = Status::Dealt | Status::Assigned
	virtual bool getCurrentPlayer(Player& player) const final // const
	{
		if ((Status::Dealt <= this->status && this->status <= Status::Started && /* 0 <= this->currentPlayer && */this->currentPlayer < this->players.size()) || Status::Over == this->status)
		{
			player = this->currentPlayer;
			return true;
		}
		else
			return false;
	}
	virtual bool setLandlord(const bool b) { UNREFERENCED_PARAMETER(b); return false; } // records[0], currentPlayer, dealer (const) -> dealer, lastHand -> lastHand (reset), amounts[0], and status (const) -> status = Status::Assigned
	virtual bool setLandlord(const Score score) { UNREFERENCED_PARAMETER(score); return false; } // records[0], currentPlayer, dealer (const) -> dealer, lastHand -> lastHand (reset), amounts[0], and status (const) -> status = Status::Assigned
	virtual bool start(const std::vector<Card>& cards, std::vector<Candidate>& candidates) final // records[1], currentPlayer, lastHand, amounts (const) | amounts[0] | amounts(this->players.size()), and status = Status::Started | Status::Over
	{
		if (Status::Assigned == this->status && this->records.size() == 1 && !this->records[0].empty() && /* 0 <= this->currentPlayer && */this->currentPlayer < this->players.size() && this->checkStarting(cards))
		{
			Hand hand{ this->currentPlayer, cards };
			if (this->processHand(hand, candidates) && this->removeCards(cards, this->players[this->currentPlayer]))
			{
				this->records.push_back(std::vector<Hand>{ hand });
				this->status = Status::Started;
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
				}
				return true;
			}
			else
				return false;
		}
		else
			return false;

	}
	virtual bool start(const std::string& description, std::vector<Candidate>& candidates) final // records[1], currentPlayer, lastHand, amounts (const) | amounts[0] | amounts(this->players.size()), and status = Status::Started | Status::Over
	{
		if (Status::Assigned == this->status && this->records.size() == 1 && !this->records[0].empty())
		{
			std::vector<Card> cards{};
			return this->description2cards(description, cards) && this->start(cards, candidates);
		}
		else
			return false;
	}
	virtual bool play(const std::vector<Card>& cards, std::vector<Candidate>& candidates) final // records, currentPlayer, lastHand, amounts (const) | amounts[0] | amounts(this->players.size()), and status (const) -> status = Status::Over
	{
		if (Status::Started == this->status && this->records.size() >= 2 && !this->records.back().empty() && /* 0 <= this->currentPlayer && */this->currentPlayer < this->players.size() && this->lastHand)
		{
			Hand hand{ this->currentPlayer, cards };
			if (this->processHand(hand, candidates))
				if (hand.player == this->lastHand.player)
					if (Type::Empty != hand.type && this->removeCards(cards, this->players[this->currentPlayer]))
						if (this->coverLastHand(hand))
							this->records.back().push_back(hand);
						else
							this->records.push_back(std::vector<Hand>{ hand });
					else
						return false;
				else if (Type::Empty == hand.type)
				{
					this->records.back().push_back(hand);
					this->nextPlayer();
					this->processBasis(hand);
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
	virtual bool play(const std::string& description, std::vector<Candidate>& candidates) final // records, currentPlayer, lastHand, amounts (const) | amounts[0] | amounts(this->players.size()), and status (const) -> status = Status::Over
	{
		if (Status::Started == this->status && this->records.size() >= 2 && !this->records.back().empty())
		{
			std::vector<Card> cards{};
			return this->description2cards(description, cards) && this->play(cards, candidates);
		}
		else
			return false;
	}
	virtual bool set(const std::vector<char>& binaryChars) final // values, players, deck, records, currentPlayer, dealer, lastHand, amounts, and status
	{
		const size_t length = binaryChars.size(), playerCount = this->players.size();
		char keyChar = 0;
		std::vector<char> valueBuffer{};
		std::vector<Card> cards{};
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
				const Player player = (Player)(keyChar & 0xF/* 0b1111 */);
				if (/* 0 <= player && */player < playerCount)
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
				std::vector<Candidate> candidates{};
				if (this->start(cards, candidates))
					break;
				else
					return false;
			}
			case 'P':
			case 'p':
			{
				std::vector<Candidate> candidates{};
				if (this->play(cards, candidates))
					break;
				else
					return false;
			}
			}
		}
		return true;
	}
	virtual bool display(const std::vector<Player>& selectedPlayers) const = 0; // const
	virtual bool display() const final // const
	{
		const size_t playerCount = this->players.size();
		std::vector<Player> selectedPlayers(playerCount);
		for (Player player = 0; player < playerCount; ++player)
			selectedPlayers[player] = player;
		return this->display(selectedPlayers);
	}
	virtual bool display(const Player player) const final { return INVALID_PLAYER == player ? this->display() : this->display(std::vector<Player>{ player }); } // const
};

class Landlords : public Poker /* Next: LandlordsX */
{
public:
	static const size_t MinimumPlayerCount = 3, MaximumPlayerCount = 3, CardCountPerPlayer = 17;
	
private:
	bool checkPlayerCount(const size_t playerCount) const override final
	{
		return Landlords::MinimumPlayerCount <= playerCount && playerCount <= Landlords::MaximumPlayerCount;
	}
	bool checkStarting(const std::vector<Card>& cards) const override final
	{
		return !cards.empty();
	}
	bool isRealHand(const Hand& hand) const override final
	{
		switch (hand.type)
		{
		case Type::Single: // 单牌
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
		case Type::PairJokers: // 王炸/火箭
		case Type::Quadruple: // 炸弹
			return !hand.cards.empty();
		case Type::Empty:
		case Type::SingleFlush:
		case Type::SingleFlushStraight:
		case Type::PairStraightWithSingle:
		case Type::TripleWithPairSingle:
		case Type::TripleStraightWithSingle:
		case Type::QuadrupleWithSingle:
		case Type::QuadrupleStraight:
		case Type::QuadrupleStraightWithSingle:
		case Type::QuadrupleJokers:
		case Type::Quintuple:
		case Type::Sextuple:
		case Type::Septuple:
		case Type::Octuple:
		case Type::Invalid:
		default:
			return false;
		}
	}
	bool processBasis(const Hand& hand) override final
	{
		if (Status::Started == this->status && !this->records.empty() && !this->records.back().empty() && this->amounts.size() == 1)
		{
			if (Type::Single <= hand.type && hand.type <= Type::QuadrupleWithPairPair && !hand.cards.empty())
			{
				if (hand.player != this->dealer)
					this->amounts[0] &= 0x1FFD/* 0b1111111111101 */;
				else if (!this->lastHand)
					this->amounts[0] &= 0x1FFE/* 0b1111111111110 */;
			}
			if (Type::Quadruple == hand.type || Type::PairJokers == hand.type)
				this->amounts[0] += !this->lastHand || hand.player == this->lastHand.player ? 0x4/* 0b100 */ : 0x40/* 0b1000000 */;
			return true;
		}
		else
			return false;
	}
	bool computeAmounts(const unsigned char multiplication1Opening7, const unsigned int basis12Calling4Robbing4Real4Empty4Spring4) override final
	{
		if (Status::Over == this->status && this->players.size() == 3)
		{
			switch (this->amounts.size())
			{
			case 1: // at most 5 ** 3 * 4096 * 5 ** 18 * 2 = 3906250000000000000 < 1 << 63 = 9223372036854775808
			{
				if (this->amounts[0] < 0)
					return false;
				const Amount baseline = this->amounts[0];
				Amount mutableAmounts[5] = { /* calling = */ (baseline >> 12) & 0x1/* 0b1 */, /* robbing = */ (baseline >> 10) & 0x3/* 0b11 */, /* realBooms = */ (baseline >> 6) & 0xF/* 0b1111 */, /* emptyBooms = */ (baseline >> 2) & 0xF/* 0b1111 */, /* springAntispring = */ static_cast<Amount>((bool)(baseline & 0x3/* 0b11 */)) };
				const Amount constAmounts[5] = { mutableAmounts[0], mutableAmounts[1], mutableAmounts[2], mutableAmounts[3], mutableAmounts[4] };
				Amount* p = mutableAmounts;
				const Amount* q = constAmounts;
				for (Count count = 0, offset = 16; count < 4; ++count)
				{
					switch ((basis12Calling4Robbing4Real4Empty4Spring4 >> offset) & 0xF/* 0b1111 */)
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
#if ((defined _MSVC_LANG && _MSVC_LANG >= 201703L) || (!defined _MSVC_LANG && defined __cplusplus && __cplusplus >= 201103L))
						[[fallthrough]];
#endif
					case 14:
						*p *= *q;
#if ((defined _MSVC_LANG && _MSVC_LANG >= 201703L) || (!defined _MSVC_LANG && defined __cplusplus && __cplusplus >= 201103L))
						[[fallthrough]];
#endif
					case 13:
						*p *= *q;
#if ((defined _MSVC_LANG && _MSVC_LANG >= 201703L) || (!defined _MSVC_LANG && defined __cplusplus && __cplusplus >= 201103L))
						[[fallthrough]];
#endif
					case 12:
						*p *= *q;
#if ((defined _MSVC_LANG && _MSVC_LANG >= 201703L) || (!defined _MSVC_LANG && defined __cplusplus && __cplusplus >= 201103L))
						[[fallthrough]];
#endif
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
					this->amounts[0] = static_cast<Amount>(multiplication1Opening7 & 0x7F/* 0b01111111*/) * (basis12Calling4Robbing4Real4Empty4Spring4 >> 20) * mutableAmounts[0] * mutableAmounts[1] * mutableAmounts[2] * mutableAmounts[3] * mutableAmounts[4];
				else
					this->amounts[0] = static_cast<Amount>(multiplication1Opening7 & 0x7F/* 0b01111111*/) + (basis12Calling4Robbing4Real4Empty4Spring4 >> 20) + mutableAmounts[0] + mutableAmounts[1] + mutableAmounts[2] + mutableAmounts[3] + mutableAmounts[4];
				this->amounts = std::vector<Amount>(4);
				Amount s = 0;
				for (Player player = 0; player < 3;)
				{
					this->amounts[static_cast<size_t>(player) + 1] = player == this->dealer ? (this->players[player].empty() ? baseline << 1 : -(baseline << 1)) : (this->players[player].empty() ? baseline : -baseline);
					s += this->amounts[++player];
				}
				if (s)
				{
					this->amounts = std::vector<Amount>{ baseline };
					return false;
				}
				else
					this->amounts[0] = baseline;
#if ((defined _MSVC_LANG && _MSVC_LANG >= 201703L) || (!defined _MSVC_LANG && defined __cplusplus && __cplusplus >= 201103L))
				[[fallthrough]];
#endif
			}
			case 4:
				return true;
			default:
				return false;
			}
		}
		else
			return false;
	}
	bool computeAmounts() override final
	{
		return this->computeAmounts(0x80/* 0b10000000 */, 0x00ACCCC/* 0b00000000101011001100110011001100 */);
	}
	bool isAbsolutelyLargest(const Hand& hand) const override final
	{
		return Type::PairJokers == hand.type;
	}
	std::string getBasisString() const override final
	{
		return this->amounts.size() >= 1 ? "倍数信息：当前共叫地主 " + std::to_string(this->amounts[0] >> 12) + " 次，抢地主 " + std::to_string((this->amounts[0] >> 10) & 0x3/* 0b11 */) + " 次；共出实炸 " + std::to_string((this->amounts[0] >> 6) & 0xF/* 0b1111 */) + " 个，空炸 " + std::to_string((this->amounts[0] >> 2) & 0xF/* 0b1111 */) + " 个；春天" + ((this->amounts[0] >> 1) & 0x1/* 0b1 */ ? "未" : "已") + "解除，反春天" + (this->amounts[0] & 0x1/* 0b1 */ ? "未" : "已") + "解除。\n" : "";
	}
	std::string getPreRoundString() const override final
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
			if (0 == callerAndRobberCount && length >= 3)
				return "无人叫地主，强制玩家 " + std::to_string(this->records[0][0].player + 1) + " 为地主。";
			else
			{
				std::string preRoundString{};
				bool isRobbing = false;
				for (size_t idx = 0; idx < length; ++idx)
				{
					const std::string playerString = std::to_string(this->records[0][idx].player + 1);
					if (this->records[0][idx].cards.empty())
						preRoundString += (isRobbing ? "不抢（玩家 " : "不叫（玩家 ") + playerString + "） -> ";
					else if (isRobbing)
						preRoundString += "抢地主（玩家 " + playerString + "） -> ";
					else
					{
						preRoundString += "叫地主（玩家 " + playerString + "） -> ";
						isRobbing = true;
					}
				}
				preRoundString.erase(preRoundString.length() - 4, 4);
				return preRoundString;
			}
		}
	}
	
protected:
	bool coverLastHand(const Hand& currentHand) const override final
	{
		if (this->isRealHand(this->lastHand) && this->isRealHand(currentHand))
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
			case Type::Empty:
			case Type::SingleFlush:
			case Type::SingleFlushStraight:
			case Type::PairStraightWithSingle:
			case Type::TripleWithPairSingle:
			case Type::TripleStraightWithSingle:
			case Type::QuadrupleWithSingle:
			case Type::QuadrupleStraight:
			case Type::QuadrupleStraightWithSingle:
			case Type::QuadrupleJokers:
			case Type::Quintuple:
			case Type::Sextuple:
			case Type::Septuple:
			case Type::Octuple:
			case Type::Invalid:
			default:
				return false;
			}
		else
			return false;
	}
	bool processHand(Hand& hand, std::vector<Candidate>& candidates) const override
	{
		hand.type = Type::Invalid;
		candidates.clear();
		bool blackJoker = false, redJoker = false;
		std::vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (JOKER_POINT == card.point)
				switch (card.suit)
				{
				case Suit::Black:
					if (blackJoker)
						return false;
					else
					{
						blackJoker = true;
						++counts[JOKER_POINT];
						break;
					}
				case Suit::Red:
					if (redJoker)
						return false;
					else
					{
						redJoker = true;
						++counts[JOKER_POINT];
						break;
					}
				case Suit::Diamond:
				case Suit::Club:
				case Suit::Heart:
				case Suit::Spade:
				case Suit::Cover:
				default:
					return false;
				}
			else if (this->values[card.point])
				++counts[card.point];
			else
				return false;
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && (valueA > valueB || (valueA == valueB && a.suit > b.suit))); });
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
			default:
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
						{
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
						{
							if (this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
								return false;
							else if (this->values[hand.cards[9].point] + 3 == this->values[hand.cards[0].point])
							{
								if (this->values[hand.cards[12].point] < this->values[hand.cards[15].point])
									rotate(hand.cards.begin() + 12, hand.cards.begin() + 15, hand.cards.end()); // e.g., AAAKKKQQQJJJ3335 -> AAAKKKQQQJJJ + 5333
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								return true;
							}
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
							{
								if (this->values[hand.cards[15].point] + 5 == this->values[hand.cards[0].point])
									return false;
								else if (this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
								{
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
								}
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
	
public:
	Landlords() : Poker()
	{
		this->name = "斗地主";
	}
	bool initialize() override final
	{
		if (this->status >= Status::Ready)
		{
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->values.set(JOKER_POINT, value++);
			this->players = std::vector<std::vector<Card>>(3);
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
	bool deal() override final
	{
		const size_t playerCount = this->players.size();
		if (this->status >= Status::Initialized && this->checkPlayerCount(playerCount))
		{
			this->deck.clear();
			this->add54CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < playerCount; ++player)
			{
				this->players[player] = std::vector<Card>(Landlords::CardCountPerPlayer);
				for (size_t idx = 0; idx < Landlords::CardCountPerPlayer; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records = std::vector<std::vector<Hand>>{ std::vector<Hand>{} };
			std::uniform_int_distribution<size_t> distribution(0, playerCount - 1);
			this->currentPlayer = (Player)(distribution(this->seed));
			this->dealer = INVALID_PLAYER;
			this->lastHand = Hand{};
			this->amounts = std::vector<Amount>{ 0x0/* 0b0 */ };
			this->status = Status::Dealt;
			return true;
		}
		else
			return false;
	}
	bool setLandlord(const bool b) override final
	{
		if (Status::Dealt == this->status && this->records.size() == 1 && /* 0 <= this->currentPlayer && */this->currentPlayer < this->players.size() && this->amounts.size() == 1)
			switch (this->records[0].size())
			{
			case 0:
				if (b)
				{
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{ Card{} } });
					this->lastHand = this->records[0][0];
					this->amounts[0] = 0x1003; // 0b1000000000011
				}
				else
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{} });
				this->nextPlayer();
				return true;
			case 1:
				if (b)
				{
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{ Card{} } });
					if (this->lastHand)
						this->amounts[0] += 0x400; // 0b10000000000
					else
					{
						this->lastHand = this->records[0][1];
						this->amounts[0] = 0x1003; // 0b1000000000011
					}
				}
				else
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{} });
				this->nextPlayer();
				return true;
			case 2:
			{
				if (b)
				{
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{ Card{} } });
					if (this->lastHand)
						this->amounts[0] += 0x400; // 0b10000000000
					else
						this->amounts[0] = 0x1003; // 0b1000000000011
				}
				else
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{} });
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
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{ Card{} } });
					if (this->lastHand)
						this->amounts[0] += 0x400; // 0b10000000000
					else
						return false;
				}
				else
				{
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{} });
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
	bool display(const std::vector<Player>& selectedPlayers) const override final
	{
		return this->status >= Status::Assigned ? Poker::display(selectedPlayers, "地主", "地主牌：" + this->cards2string(this->deck, "", " | ", "（已公开）", "（空）") + "\n\n") : Poker::display(selectedPlayers, "拥有明牌", "地主牌：" + this->cards2string(this->deck, "", " | ", "（未公开）", "（空）") + "\n\n");
	}
};

class LandlordsX : public Landlords /* Previous: Landlords | Next: Landlord4P */
{
protected:
	bool processHand(Hand& hand, std::vector<Candidate>& candidates) const override final
	{
		hand.type = Type::Invalid;
		bool blackJoker = false, redJoker = false;
		std::vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (JOKER_POINT == card.point)
				switch (card.suit)
				{
				case Suit::Black:
					if (blackJoker)
					{
						candidates.clear();
						return false;
					}
					else
					{
						blackJoker = true;
						++counts[JOKER_POINT];
						break;
					}
				case Suit::Red:
					if (redJoker)
					{
						candidates.clear();
						return false;
					}
					else
					{
						redJoker = true;
						++counts[JOKER_POINT];
						break;
					}
				case Suit::Diamond:
				case Suit::Club:
				case Suit::Heart:
				case Suit::Spade:
				case Suit::Cover:
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
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && (valueA > valueB || (valueA == valueB && a.suit > b.suit))); });
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
				std::vector<Candidate> potentialHands{};
				potentialHands.emplace_back(hand, "解析为点数为 " + this->point2description(hand.cards[0].point) + " 的炸弹（``Type::Quadruple``）");
				potentialHands.back().hand.type = Type::Quadruple; // 炸弹
				potentialHands.emplace_back(hand, "解析为点数为 " + this->point2description(hand.cards[0].point) + " 的三带一（``Type::TripleWithSingle``）");
				potentialHands.back().hand.type = Type::TripleWithSingle; // 三带一
				if (this->lastHand && hand.player != this->lastHand.player)
					for (std::vector<Candidate>::iterator it = potentialHands.begin(); it != potentialHands.end();)
					{
						if (this->coverLastHand(it->hand))
							++it;
						else
							it = potentialHands.erase(it);
					}
				switch (potentialHands.size())
				{
				case 0:
					candidates.clear();
					return false;
				case 1:
					hand = std::move(potentialHands[0].hand);
					candidates.clear();
					return true;
				default:
					if (candidates.size() == 1)
					{
						const std::vector<Candidate>::iterator it = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
						if (it != potentialHands.end())
						{
							hand = std::move(it->hand);
							candidates.clear();
							return true;
						}
					}
					candidates = std::move(potentialHands);
					return false;
				}
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
			default:
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
					std::vector<Candidate> potentialHands{};
					const std::string cardString0 = this->point2description(hand.cards[0].point), cardString4 = this->point2description(hand.cards[4].point);
					if (value <= 12 && this->values[hand.cards[4].point] + 1 == value)
					{
						potentialHands.emplace_back(hand, "解析为长度为 2 且点数为 " + cardString0 + " 的飞机带小翼（``Type::TripleStraightWithSingles``）");
						rotate(potentialHands.back().hand.cards.begin() + 3, potentialHands.back().hand.cards.begin() + 4, potentialHands.back().hand.cards.begin() + 7);
						potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
					}
					potentialHands.emplace_back(hand, "解析为点数为 " + cardString0 + " 的四带二对（``Type::QuadrupleWithPairPair``），其中被带的牌包含两个点数为 " + cardString4 + " 的对子");
					potentialHands.back().hand.type = Type::QuadrupleWithPairPair; // 四带二对
					potentialHands.emplace_back(hand, "解析为点数为 " + cardString4 + " 的四带二对（``Type::QuadrupleWithPairPair``），其中被带的牌包含两个点数为 " + cardString0 + " 的对子");
					rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 4, potentialHands.back().hand.cards.end());
					potentialHands.back().hand.type = Type::QuadrupleWithPairPair; // 四带二对
					if (this->lastHand && hand.player != this->lastHand.player)
						for (std::vector<Candidate>::iterator it = potentialHands.begin(); it != potentialHands.end();)
						{
							if (this->coverLastHand(it->hand))
								++it;
							else
								it = potentialHands.erase(it);
						}
					switch (potentialHands.size())
					{
					case 0:
						candidates.clear();
						return false;
					case 1:
						hand = std::move(potentialHands[0].hand);
						candidates.clear();
						return true;
					default:
						if (candidates.size() == 1)
						{
							const std::vector<Candidate>::iterator it = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
							if (it != potentialHands.end())
							{
								hand = std::move(it->hand);
								candidates.clear();
								return true;
							}
						}
						candidates = std::move(potentialHands);
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
						{
							if (this->values[hand.cards[9].point] + 3 == this->values[hand.cards[0].point])
							{
								std::vector<Candidate> potentialHands{};
								const std::string cardString0 = this->point2description(hand.cards[0].point), cardString3 = this->point2description(hand.cards[3].point), cardString9 = this->point2description(hand.cards[9].point);
								potentialHands.emplace_back(hand, "解析为长度为 4 且点数为 " + cardString0 + " 的飞机（``Type::TripleStraight``）");
								potentialHands.back().hand.type = Type::TripleStraight; // 飞机（不带翅膀）
								potentialHands.emplace_back(hand, "解析为长度为 3 且点数为 " + cardString0 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼的点数均为 " + cardString9);
								potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								potentialHands.emplace_back(hand, "解析为长度为 3 且点数为 " + cardString3 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼的点数均为 " + cardString0);
								rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, potentialHands.back().hand.cards.end());
								potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								if (this->lastHand && hand.player != this->lastHand.player)
									for (std::vector<Candidate>::iterator it = potentialHands.begin(); it != potentialHands.end();)
									{
										if (this->coverLastHand(it->hand))
											++it;
										else
											it = potentialHands.erase(it);
									}
								switch (potentialHands.size())
								{
								case 0:
									candidates.clear();
									return false;
								case 1:
									hand = std::move(potentialHands[0].hand);
									candidates.clear();
									return true;
								default:
									if (candidates.size() == 1)
									{
										const std::vector<Candidate>::iterator it = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
										if (it != potentialHands.end())
										{
											hand = std::move(it->hand);
											candidates.clear();
											return true;
										}
									}
									candidates = std::move(potentialHands);
									return false;
								}
							}
							else if (this->values[hand.cards[6].point] + 2 == this->values[hand.cards[0].point])
							{
								hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								candidates.clear();
								return true;
							}
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
							std::vector<Card>::iterator cardIt = hand.cards.begin() + 3;
							const std::vector<Card>::iterator indexToLastBodyPoint = hand.cards.begin() + 12;
							const Value value = this->values[hand.cards[0].point];
							while (cardIt <= indexToLastBodyPoint && this->values[cardIt->point] > value)
								cardIt += 3;
							rotate(hand.cards.begin(), hand.cards.begin() + 3, cardIt);
							if (this->values[hand.cards[0].point] <= 12)
							{
								if (this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
								{
									std::vector<Candidate> potentialHands{};
									const std::string cardString0 = this->point2description(hand.cards[0].point), cardString3 = this->point2description(hand.cards[3].point), cardString12 = this->point2description(hand.cards[12].point);
									potentialHands.emplace_back(hand, "解析为长度为 4 且点数为 " + cardString0 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + cardString12 + " 的牌");
									if (this->values[hand.cards[12].point] < this->values[hand.cards[15].point])
										rotate(potentialHands.back().hand.cards.begin() + 12, potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.end()); // e.g., 9999888777666555 -> 9998887776665559 -> 999888777666 + 9555 | else e.g., 5555999888777666 -> 9998887776665555 -> 999888777666 + 5555
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									potentialHands.emplace_back(hand, "解析为长度为 4 且点数为 " + cardString3 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + cardString0 + " 的牌");
									rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, potentialHands.back().hand.cards.end() - 1); // e.g., 8888999777666555 -> 9998887776665558 -> 888777666555 + 9998
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									if (this->lastHand && hand.player != this->lastHand.player)
										for (std::vector<Candidate>::iterator candidateIt = potentialHands.begin(); candidateIt != potentialHands.end();)
										{
											if (this->coverLastHand(candidateIt->hand))
												++candidateIt;
											else
												candidateIt = potentialHands.erase(candidateIt);
										}
									switch (potentialHands.size())
									{
									case 0:
										candidates.clear();
										return false;
									case 1:
										hand = std::move(potentialHands[0].hand);
										candidates.clear();
										return true;
									default:
										if (candidates.size() == 1)
										{
											const std::vector<Candidate>::iterator candidateIt = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
											if (candidateIt != potentialHands.end())
											{
												hand = candidateIt->hand;
												candidates.clear();
												return true;
											}
										}
										candidates = std::move(potentialHands);
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
							std::vector<Card>::iterator it = hand.cards.begin() + 3;
							const std::vector<Card>::iterator indexToLastBodyPoint = hand.cards.begin() + 9;
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
						{
							if (this->values[hand.cards[12].point] + 4 == this->values[hand.cards[0].point])
							{
								std::vector<Candidate> potentialHands{};
								const std::string cardString0 = this->point2description(hand.cards[0].point), cardString3 = this->point2description(hand.cards[3].point), cardString12 = this->point2description(hand.cards[12].point);
								potentialHands.emplace_back(hand, "解析为长度为 4 且点数为 " + cardString0 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + cardString12 + " 的牌");
								if (this->values[hand.cards[12].point] < this->values[hand.cards[15].point])
									rotate(potentialHands.back().hand.cards.begin() + 12, potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.end()); // e.g., 999888777666555K -> 999888777666 + K555
								potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								potentialHands.emplace_back(hand, "解析为长度为 4 且点数为 " + cardString3 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + cardString0 + " 的牌");
								rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, hand.cards[0].point < hand.cards[15].point ? potentialHands.back().hand.cards.end() : potentialHands.back().hand.cards.end() - 1); // e.g., 999888777666555K -> 888777666555 + K999 | 9998887776665553 -> 888777666555 + 9993
								potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
								if (this->lastHand && hand.player != this->lastHand.player)
								{
									for (std::vector<Candidate>::iterator it = potentialHands.begin(); it != potentialHands.end();)
										if (this->coverLastHand(it->hand))
											++it;
										else
											it = potentialHands.erase(it);
								}
								switch (potentialHands.size())
								{
								case 0:
									candidates.clear();
									return false;
								case 1:
									hand = std::move(potentialHands[0].hand);
									candidates.clear();
									return true;
								default:
									if (candidates.size() == 1)
									{
										const std::vector<Candidate>::iterator it = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
										if (it != potentialHands.end())
										{
											hand = std::move(it->hand);
											candidates.clear();
											return true;
										}
									}
									candidates = std::move(potentialHands);
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
								std::vector<Card>::iterator it = hand.cards.begin() + 12;
								const std::vector<Card>::iterator indexToSecondBodyPoint = hand.cards.begin() + 3;
								const Value value12 = this->values[hand.cards[12].point];
								while (it >= indexToSecondBodyPoint && this->values[(it - 3)->point] < value12)
									it -= 3;
								rotate(it, hand.cards.begin() + 12, hand.cards.begin() + 15);
								const Value value0 = this->values[hand.cards[0].point];
								if (value0 <= 12 && this->values[hand.cards[12].point] + 4 == value0)
								{
									const std::vector<Card>::iterator indexToSecondSidePoint = hand.cards.begin() + 16;
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
								std::vector<Card>::iterator it = hand.cards.begin() + 9;
								const std::vector<Card>::iterator indexToSecondBodyPoint = hand.cards.begin() + 3;
								const Value value9 = this->values[hand.cards[9].point], value12 = this->values[hand.cards[12].point];
								while (it >= indexToSecondBodyPoint && this->values[(it - 3)->point] < value9)
									it -= 3;
								rotate(it, hand.cards.begin() + 9, hand.cards.begin() + 12);
								it += 3;
								std::vector<Card>::iterator secondaryIt = hand.cards.begin() + 12;
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
								rotate(hand.cards.begin() + 7, hand.cards.begin() + 8, hand.cards.end());
								rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end() - 1);
								std::vector<Card>::iterator it = hand.cards.begin() + 3, secondaryIt = hand.cards.begin();
								const std::vector<Card>::iterator indexToLastBodyPoint = hand.cards.begin() + 15;
								const Value originalValue0 = this->values[hand.cards[0].point], originalValue3 = this->values[hand.cards[3].point];
								while (it <= indexToLastBodyPoint && this->values[it->point] > originalValue3)
									it += 3;
								rotate(hand.cards.begin() + 3, hand.cards.begin() + 6, it);
								it -= 3;
								while (secondaryIt < it && this->values[secondaryIt->point] > originalValue0)
									secondaryIt += 3;
								rotate(hand.cards.begin(), hand.cards.begin() + 3, secondaryIt);
								const Value newValue0 = this->values[hand.cards[0].point];
								const std::string cardString0 = this->point2description(hand.cards[0].point), cardString3 = this->point2description(hand.cards[3].point), cardString15 = this->point2description(hand.cards[15].point);
								if (newValue0 <= 12)
								{
									if (this->values[hand.cards[15].point] + 5 == newValue0)
									{
										std::vector<Candidate> potentialHands{};
										if (hand.cards[18].point == hand.cards[0].point && hand.cards[19].point == hand.cards[3].point)
										{
											const std::string cardString6 = this->point2description(hand.cards[6].point);
											potentialHands.emplace_back(hand, "解析为长度为 4 且点数为 " + cardString6 + " 的飞机带大翼（``Type::TripleStraightWithPairs``），其中大翼为两个点数为 " + cardString0 + " 的对子以及两个点数为 " + cardString3 + " 的对子");
											rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, potentialHands.back().hand.cards.end() - 2);
											rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, potentialHands.back().hand.cards.end() - 1);
											potentialHands.back().hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
										}
										else if (hand.cards[18].point == hand.cards[3].point && hand.cards[19].point == hand.cards[15].point)
										{
											potentialHands.emplace_back(hand, "解析为长度为 4 且点数为 " + cardString3 + " 的飞机带大翼（``Type::TripleStraightWithPairs``），其中大翼为两个点数为 " + cardString0 + " 的对子以及两个点数为 " + cardString15 + " 的对子");
											rotate(potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.begin() + 18, potentialHands.back().hand.cards.begin() + 19);
											rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, potentialHands.back().hand.cards.begin() + 15);
											potentialHands.back().hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
										}
										else if (hand.cards[18].point == hand.cards[12].point && hand.cards[19].point == hand.cards[15].point)
										{
											const std::string cardString12 = this->point2description(hand.cards[12].point);
											potentialHands.emplace_back(hand, "解析为长度为 4 且点数为 " + cardString0 + " 的飞机带大翼（``Type::TripleStraightWithPairs``），其中大翼为两个点数为 " + cardString12 + " 的对子以及两个点数为 " + cardString15 + " 的对子");
											rotate(potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.begin() + 18, potentialHands.back().hand.cards.begin() + 19);
											potentialHands.back().hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
										}
										potentialHands.emplace_back(hand, "解析为长度为 5 且点数为 " + cardString0 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + cardString15 + " 的牌");
										rotate(potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.begin() + 18, this->values[hand.cards[15].point] < this->values[hand.cards[19].point] ? potentialHands.back().hand.cards.end() : potentialHands.back().hand.cards.end() - 1); // e.g., 77776666999888555444 -> 99988877766655544476 -> 999888777666555 + 76444 | 55554444999888777666 -> 99988877766655544454 -> 999888777666555 + 54444
										potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
										potentialHands.emplace_back(hand, "解析为长度为 5 且点数为 " + cardString3 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + cardString0 + " 的牌");
										rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, potentialHands.back().hand.cards.end() - 2); // e.g., 77776666999888555444 -> 99988877766655544476 -> 888777666555444 + 99976
										potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
										if (this->lastHand && hand.player != this->lastHand.player)
											for (std::vector<Candidate>::iterator candidateIt = potentialHands.begin(); candidateIt != potentialHands.end();)
											{
												if (this->coverLastHand(candidateIt->hand))
													++candidateIt;
												else
													candidateIt = potentialHands.erase(candidateIt);
											}
										switch (potentialHands.size())
										{
										case 0:
											candidates.clear();
											return false;
										case 1:
											hand = std::move(potentialHands[0].hand);
											candidates.clear();
											return true;
										default:
											if (candidates.size() == 1)
											{
												const std::vector<Candidate>::iterator candidateIt = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
												if (candidateIt != potentialHands.end())
												{
													hand = candidateIt->hand;
													candidates.clear();
													return true;
												}
											}
											candidates = std::move(potentialHands);
											return false;
										}
									}
									else if (this->values[hand.cards[12].point] + 4 == newValue0)
										if (hand.cards[18].point == hand.cards[3].point && hand.cards[19].point == hand.cards[15].point)
										{
											std::vector<Candidate> potentialHands{};
											potentialHands.emplace_back(hand, "解析为长度为 4 且点数为 " + cardString3 + " 的飞机带大翼（``Type::TripleStraightWithPairs``），其中大翼为两个点数为 " + cardString0 + " 的对子以及两个点数为 " + cardString15 + " 的对子");
											rotate(potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.begin() + 18, potentialHands.back().hand.cards.begin() + 19);
											rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, potentialHands.back().hand.cards.begin() + 15);
											potentialHands.back().hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
											potentialHands.emplace_back(hand, "解析为长度为 5 且点数为 " + cardString0 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + cardString15 + " 的牌");
											rotate(potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.begin() + 18, this->values[hand.cards[15].point] < this->values[hand.cards[19].point] ? potentialHands.back().hand.cards.end() : potentialHands.back().hand.cards.end() - 1); // e.g., 777766666999888555333 -> 99988877766655533376 -> 999888777666555 + 76333 | 55553333999888777666 -> 99988877766655533353 -> 999888777666555 + 53333
											potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
											if (this->lastHand && hand.player != this->lastHand.player)
												for (std::vector<Candidate>::iterator candidateIt = potentialHands.begin(); candidateIt != potentialHands.end();)
												{
													if (this->coverLastHand(candidateIt->hand))
														++candidateIt;
													else
														candidateIt = potentialHands.erase(candidateIt);
												}
											switch (potentialHands.size())
											{
											case 0:
												candidates.clear();
												return false;
											case 1:
												hand = std::move(potentialHands[0].hand);
												candidates.clear();
												return true;
											default:
												if (candidates.size() == 1)
												{
													const std::vector<Candidate>::iterator candidateIt = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
													if (candidateIt != potentialHands.end())
													{
														hand = candidateIt->hand;
														candidates.clear();
														return true;
													}
												}
												candidates = std::move(potentialHands);
												return false;
											}
										}
										else
										{
											rotate(hand.cards.begin() + 15, hand.cards.begin() + 18, this->values[hand.cards[15].point] < this->values[hand.cards[19].point] ? hand.cards.end() : hand.cards.end() - 1); // e.g., 777766666999888555333 -> 99988877766655533376 -> 999888777666555 + 76333 | 55553333999888777666 -> 99988877766655533353 -> 999888777666555 + 53333
											hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
											candidates.clear();
											return true;
										}
									else if (this->values[hand.cards[9].point] + 3 == newValue0 && hand.cards[18].point == hand.cards[12].point && hand.cards[19].point == hand.cards[15].point)
									{
										rotate(hand.cards.begin() + 15, hand.cards.begin() + 16, hand.cards.end() - 1);
										hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
										candidates.clear();
										return true;
									}
								}
								const Value newValue3 = this->values[hand.cards[3].point], value15 = this->values[hand.cards[15].point];
								if (value15 + 4 == newValue3)
									if (hand.cards[18].point == hand.cards[3].point && hand.cards[19].point == hand.cards[15].point)
									{
										std::vector<Candidate> potentialHands{};
										potentialHands.emplace_back(hand, "解析为长度为 4 且点数为 " + cardString3 + " 的飞机带大翼（``Type::TripleStraightWithPairs``），其中大翼为两个点数为 " + cardString0 + " 的对子以及两个点数为 " + cardString15 + " 的对子");
										rotate(potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.begin() + 18, potentialHands.back().hand.cards.begin() + 19);
										rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, potentialHands.back().hand.cards.begin() + 15);
										potentialHands.back().hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
										potentialHands.emplace_back(hand, "解析为长度为 5 且点数为 " + cardString3 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + cardString0 + " 的牌");
										rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, potentialHands.back().hand.cards.end() - 2); // e.g., 77776666JJJ888555444 -> JJJ88877766655544476 -> 888777666555444 + JJJ76
										potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
										if (this->lastHand && hand.player != this->lastHand.player)
											for (std::vector<Candidate>::iterator candidateIt = potentialHands.begin(); candidateIt != potentialHands.end();)
											{
												if (this->coverLastHand(candidateIt->hand))
													++candidateIt;
												else
													candidateIt = potentialHands.erase(candidateIt);
											}
										switch (potentialHands.size())
										{
										case 0:
											candidates.clear();
											return false;
										case 1:
											hand = std::move(potentialHands[0].hand);
											candidates.clear();
											return true;
										default:
											if (candidates.size() == 1)
											{
												const std::vector<Candidate>::iterator candidateIt = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
												if (candidateIt != potentialHands.end())
												{
													hand = candidateIt->hand;
													candidates.clear();
													return true;
												}
											}
											candidates = std::move(potentialHands);
											return false;
										}
									}
									else
									{
										rotate(hand.cards.begin(), hand.cards.begin() + 3, hand.cards.end() - 2); // e.g., 77776666JJJ888555444 -> JJJ88877766655544476 -> 888777666555444 + JJJ76
										hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
										candidates.clear();
										return true;
									}
								else if (this->values[hand.cards[12].point] + 3 == newValue3)
								{
									candidates.clear();
									if (hand.cards[18].point == hand.cards[0].point && hand.cards[19].point == hand.cards[15].point)
									{
										rotate(hand.cards.begin() + 15, hand.cards.begin() + 18, hand.cards.begin() + 19);
										rotate(hand.cards.begin(), hand.cards.begin() + 3, hand.cards.begin() + 15);
										hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
										return true;
									}
									else
										return false;
								}
								else if (value15 + 3 == this->values[hand.cards[6].point])
								{
									candidates.clear();
									if (hand.cards[18].point == hand.cards[0].point && hand.cards[19].point == hand.cards[3].point)
									{
										rotate(hand.cards.begin(), hand.cards.begin() + 3, hand.cards.end() - 2);
										rotate(hand.cards.begin(), hand.cards.begin() + 3, hand.cards.end() - 1);
										hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
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
							}
							else
							{
								candidates.clear();
								rotate(hand.cards.begin() + 7, hand.cards.begin() + 8, hand.cards.end() - 3);
								rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end() - 4);
								std::vector<Card>::iterator it = hand.cards.begin() + 6;
								const std::vector<Card>::iterator indexToLastBodyPoint = hand.cards.begin() + 12;
								const Value originalValue = this->values[hand.cards[0].point], value3 = this->values[hand.cards[3].point];
								while (it <= indexToLastBodyPoint && this->values[it->point] > value3)
									it += 3;
								rotate(hand.cards.begin() + 3, hand.cards.begin() + 6, it);
								std::vector<Card>::iterator secondaryIt = hand.cards.begin() + 3;
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
							std::vector<Card>::iterator it = hand.cards.begin() + 3;
							const std::vector<Card>::iterator indexToLastBodyPoint = hand.cards.begin() + 15;
							while (it <= indexToLastBodyPoint && this->values[it->point] > originalValue)
								it += 3;
							rotate(hand.cards.begin(), hand.cards.begin() + 3, it);
							const Value newValue = this->values[hand.cards[0].point];
							if (newValue <= 12)
							{
								if (this->values[hand.cards[15].point] + 5 == newValue)
								{
									std::vector<Candidate> potentialHands{};
									const std::string cardString0 = this->point2description(hand.cards[0].point), cardString3 = this->point2description(hand.cards[3].point), cardString15 = this->point2description(hand.cards[15].point);
									potentialHands.emplace_back(hand, "解析为长度为 5 且点数为 " + cardString0 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + cardString15 + " 的牌");
									const Value value15 = this->values[hand.cards[15].point];
									if (value15 < this->values[hand.cards[18].point])
										rotate(potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.begin() + 18, value15 < this->values[hand.cards[19].point] ? potentialHands.back().hand.cards.end() : potentialHands.back().hand.cards.end() - 1);
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									potentialHands.emplace_back(hand, "解析为长度为 5 且点数为 " + cardString3 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + cardString0 + " 的牌");
									rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, isValue19Larger ? potentialHands.back().hand.cards.end() - 1 : potentialHands.back().hand.cards.end() - 2);
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									if (this->lastHand && hand.player != this->lastHand.player)
										for (std::vector<Candidate>::iterator candidateIt = potentialHands.begin(); candidateIt != potentialHands.end();)
										{
											if (this->coverLastHand(candidateIt->hand))
												++candidateIt;
											else
												candidateIt = potentialHands.erase(candidateIt);
										}
									switch (potentialHands.size())
									{
									case 0:
										candidates.clear();
										return false;
									case 1:
										hand = std::move(potentialHands[0].hand);
										candidates.clear();
										return true;
									default:
										if (candidates.size() == 1)
										{
											const std::vector<Candidate>::iterator candidateIt = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
											if (candidateIt != potentialHands.end())
											{
												hand = candidateIt->hand;
												candidates.clear();
												return true;
											}
										}
										candidates = std::move(potentialHands);
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
							std::vector<Candidate> potentialHands{};
							if (2 == counts[5] && 2 == counts[6] && JOKER_POINT != hand.cards[16].point && value4 <= 12 && this->values[hand.cards[13].point] + 3 == value4)
							{
								potentialHands.emplace_back(hand, "解析为长度为 4 且点数为 " + this->point2description(hand.cards[4].point) + " 的飞机带大翼（``Type::TripleStraightWithPairs``），其中大翼包含两个点数为 " + this->point2description(hand.cards[0].point) + " 的对子");
								rotate(potentialHands.back().hand.cards.begin(), potentialHands[0].hand.cards.begin() + 4, originalValue < this->values[potentialHands.back().hand.cards[16].point] ? (originalValue < this->values[potentialHands.back().hand.cards[18].point] ? potentialHands.back().hand.cards.end() : potentialHands.back().hand.cards.end() - 2) : potentialHands.back().hand.cards.end() - 4);
								potentialHands.back().hand.type = Type::TripleStraightWithPairs; // 飞机带大翼
							}
							
							/* Fetch 43333... -> 3 * 5 + 1 * 5 */
							rotate(hand.cards.begin() + 3, hand.cards.begin() + 4, hand.cards.end() - 4);
							std::vector<Card>::iterator it = hand.cards.begin() + 3;
							const std::vector<Card>::iterator indexToLastBodyPoint = hand.cards.begin() + 12;
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
									potentialHands.emplace_back(hand, "解析为长度为 5 且点数为 " + this->point2description(hand.cards[0].point) + " 的飞机带小翼（``Type::TripleStraightWithSingles``）");
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									if (this->lastHand && hand.player != this->lastHand.player)
										for (std::vector<Candidate>::iterator candidateIt = potentialHands.begin(); candidateIt != potentialHands.end();)
										{
											if (this->coverLastHand(candidateIt->hand))
												++candidateIt;
											else
												candidateIt = potentialHands.erase(candidateIt);
										}
									switch (potentialHands.size())
									{
									case 0:
										candidates.clear();
										return false;
									case 1:
										hand = std::move(potentialHands[0].hand);
										candidates.clear();
										return true;
									default:
										if (candidates.size() == 1)
										{
											const std::vector<Candidate>::iterator candidateIt = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
											if (candidateIt != potentialHands.end())
											{
												hand = candidateIt->hand;
												candidates.clear();
												return true;
											}
										}
										candidates = std::move(potentialHands);
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
								hand = std::move(potentialHands[0].hand);
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
							{
								if (this->values[hand.cards[15].point] + 5 == this->values[hand.cards[0].point])
								{
									const Value value0 = this->values[hand.cards[0].point], value15 = this->values[hand.cards[15].point];
									std::vector<Candidate> potentialHands{};
									const std::string cardString0 = this->point2description(hand.cards[0].point), cardString3 = this->point2description(hand.cards[3].point), cardString15 = this->point2description(hand.cards[15].point);
									potentialHands.emplace_back(hand, "解析为长度为 5 且点数为 " + cardString0 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + cardString15 + " 的牌");
									if (value15 < this->values[hand.cards[18].point])
										rotate(potentialHands.back().hand.cards.begin() + 15, potentialHands.back().hand.cards.begin() + 18, value15 < this->values[hand.cards[19].point] ? potentialHands.back().hand.cards.end() : potentialHands.back().hand.cards.end() - 1); // e.g., 999888777666555444KK -> 999888777666555 + KK444 | 999888777666555444K3 -> 999888777666555 + K4443
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									potentialHands.emplace_back(hand, "解析为长度为 5 且点数为 " + cardString3 + " 的飞机带小翼（``Type::TripleStraightWithSingles``），其中小翼包含三张点数为 " + cardString0 + " 的牌");
									rotate(potentialHands.back().hand.cards.begin(), potentialHands.back().hand.cards.begin() + 3, value0 < this->values[hand.cards[18].point] ? (value0 < this->values[hand.cards[19].point] ? potentialHands.back().hand.cards.end() : potentialHands.back().hand.cards.end() - 1) : potentialHands.back().hand.cards.end() - 2); // e.g., 999888777666555444KK -> 888777666555444 + KK999 | 999888777666555444K3 -> 888777666555444 + K9993 | AAAKKKQQQJJJTTT99973 -> KKKQQQJJJTTT999 + AAA73
									potentialHands.back().hand.type = Type::TripleStraightWithSingles; // 飞机带小翼
									if (this->lastHand && hand.player != this->lastHand.player)
										for (std::vector<Candidate>::iterator candidateIt = potentialHands.begin(); candidateIt != potentialHands.end();)
										{
											if (this->coverLastHand(candidateIt->hand))
												++candidateIt;
											else
												candidateIt = potentialHands.erase(candidateIt);
										}
									switch (potentialHands.size())
									{
									case 0:
										candidates.clear();
										return false;
									case 1:
										hand = std::move(potentialHands[0].hand);
										candidates.clear();
										return true;
									default:
										if (candidates.size() == 1)
										{
											const std::vector<Candidate>::iterator candidateIt = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
											if (candidateIt != potentialHands.end())
											{
												hand = candidateIt->hand;
												candidates.clear();
												return true;
											}
										}
										candidates = std::move(potentialHands);
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
		this->name = "斗地主拓展版";
	}
};

class Landlords4P : public Poker /* Previous: LandlordsX | Next: BigTwo */
{
public:
	static const size_t MinimumPlayerCount = 4, MaximumPlayerCount = 4, CardCountPerPlayer = 25;
	
private:
	bool checkPlayerCount(const size_t playerCount) const override final
	{
		return Landlords4P::MinimumPlayerCount <= playerCount && playerCount <= Landlords4P::MaximumPlayerCount;
	}
	bool checkStarting(const std::vector<Card>& cards) const override final
	{
		return !cards.empty();
	}
	bool isRealHand(const Hand& hand) const override final
	{
		switch (this->lastHand.type)
		{
		case Type::Single: // 单牌
		case Type::Pair: // 对子
		case Type::SingleStraight: // 顺子
		case Type::PairStraight: // 连对
		case Type::Triple: // 三条
		case Type::TripleWithPair: // 三带一对
		case Type::TripleStraight: // 飞机
		case Type::TripleStraightWithPairs: // 飞机带大翼
		case Type::Quadruple: // 四条炸弹
		case Type::QuadrupleJokers: // 天王炸弹
		case Type::Quintuple: // 五张炸弹
		case Type::Sextuple: // 六张炸弹
		case Type::Septuple: // 七张炸弹
		case Type::Octuple: // 八张炸弹
			return !hand.cards.empty();
		case Type::Empty:
		case Type::SingleFlush:
		case Type::SingleFlushStraight:
		case Type::PairStraightWithSingle:
		case Type::PairJokers:
		case Type::TripleWithSingle:
		case Type::TripleWithPairSingle:
		case Type::TripleStraightWithSingle:
		case Type::TripleStraightWithSingles:
		case Type::QuadrupleWithSingle:
		case Type::QuadrupleWithSingleSingle:
		case Type::QuadrupleWithPairPair:
		case Type::QuadrupleStraight:
		case Type::QuadrupleStraightWithSingle:
		case Type::Invalid:
		default:
			return false;
		}
	}
	bool coverLastHand(const Hand& currentHand) const override final
	{
		if (this->isRealHand(this->lastHand) && this->isRealHand(currentHand))
			switch (this->lastHand.type)
			{
			case Type::Single: // 单牌
			case Type::Pair: // 对子
				return currentHand.type >= Type::Quintuple || Type::Quadruple == currentHand.type || (currentHand.type == this->lastHand.type && (JOKER_POINT == currentHand.cards[0].point && JOKER_POINT == this->lastHand.cards[0].point ? currentHand.cards[0].suit > this->lastHand.cards[0].suit : this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point]));
			case Type::SingleStraight: // 顺子
			case Type::PairStraight: // 连对
			case Type::Triple: // 三条
			case Type::TripleWithPair: // 三带一对
			case Type::TripleStraight: // 飞机
			case Type::TripleStraightWithPairs: // 飞机带大翼
				return currentHand.type >= Type::Quintuple || Type::Quadruple == currentHand.type || (currentHand.type == this->lastHand.type && currentHand.cards.size() == this->lastHand.cards.size() && this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point]);
			case Type::Quadruple: // 四条炸弹
				return currentHand.type >= Type::Quintuple || (Type::Quadruple == currentHand.type && this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point]);
			case Type::QuadrupleJokers: // 天王炸弹
			case Type::Quintuple: // 五张炸弹
			case Type::Sextuple: // 六张炸弹
			case Type::Septuple: // 七张炸弹
			case Type::Octuple: // 八张炸弹
				return currentHand.type > this->lastHand.type || (currentHand.type == this->lastHand.type && this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point]);
			case Type::Empty:
			case Type::SingleFlush:
			case Type::SingleFlushStraight:
			case Type::PairStraightWithSingle:
			case Type::PairJokers:
			case Type::TripleWithSingle:
			case Type::TripleWithPairSingle:
			case Type::TripleStraightWithSingle:
			case Type::TripleStraightWithSingles:
			case Type::QuadrupleWithSingle:
			case Type::QuadrupleWithSingleSingle:
			case Type::QuadrupleWithPairPair:
			case Type::QuadrupleStraight:
			case Type::QuadrupleStraightWithSingle:
			case Type::Invalid:
			default:
				return false;
			}
		else
			return false;
	}
	bool processHand(Hand& hand, std::vector<Candidate>& candidates) const override final
	{
		hand.type = Type::Invalid;
		candidates.clear();
		Count blackJokerCount = 0, redJokerCount = 0;
		std::vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (JOKER_POINT == card.point)
			{
				switch (card.suit)
				{
				case Suit::Black:
					if (++blackJokerCount > 2)
						return false;
					else
						break;
				case Suit::Red:
					if (++redJokerCount > 2)
						return false;
					else
						break;
				case Suit::Diamond:
				case Suit::Club:
				case Suit::Heart:
				case Suit::Spade:
				case Suit::Cover:
				default:
					return false;
				}
			}
			else if (this->values[card.point])
				++counts[card.point];
			else
				return false;
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && (valueA > valueB || (valueA == valueB && a.suit > b.suit))); });
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
			default:
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
	bool processBasis(const Hand& hand) override final
	{
		if (Status::Started == this->status && this->amounts.size() == 1)
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
			case Type::Empty:
			case Type::Single:
			case Type::SingleStraight:
			case Type::SingleFlush:
			case Type::SingleFlushStraight:
			case Type::Pair:
			case Type::PairStraight:
			case Type::PairStraightWithSingle:
			case Type::PairJokers:
			case Type::Triple:
			case Type::TripleWithSingle:
			case Type::TripleWithPair:
			case Type::TripleWithPairSingle:
			case Type::TripleStraight:
			case Type::TripleStraightWithSingle:
			case Type::TripleStraightWithSingles:
			case Type::TripleStraightWithPairs:
			case Type::Quadruple:
			case Type::QuadrupleWithSingle:
			case Type::QuadrupleWithSingleSingle:
			case Type::QuadrupleWithPairPair:
			case Type::QuadrupleStraight:
			case Type::QuadrupleStraightWithSingle:
			case Type::Quintuple:
			case Type::Invalid:
			default:
				break;
			}
			return true;
		}
		else
			return false;
	}
	bool computeAmounts() override final
	{
		if (Status::Over == this->status)
		{
			const size_t recordCount = this->records.size(), playerCount = this->players.size();
			if (recordCount >= 2 && 4 == playerCount)
				switch (this->amounts.size())
				{
				case 1:
				{
					/* Winner fetching */
					Player winner = INVALID_PLAYER;
					char playerFlags[4] = { static_cast<char>(-1), static_cast<char>(-1), static_cast<char>(-1), static_cast<char>(-1) };
					for (Player player = 0; player < 4; ++player)
						if (this->players[player].empty())
						{
							if (INVALID_PLAYER == winner)
							{
								winner = player;
								playerFlags[player] = 3;
							}
							else
								return false;
						}
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
							{
								if (this->records[1][innerIdx].player == this->dealer)
									isAntiSpring = false;
								else
									isSpring = false;
							}
					}
					for (size_t outerIdx = 2; outerIdx < recordCount && (isSpring || isAntiSpring); ++outerIdx)
					{
						const size_t handCount = this->records[outerIdx].size();
						for (size_t innerIdx = 0; innerIdx < handCount; ++innerIdx)
							if (Type::Single <= this->records[outerIdx][innerIdx].type && this->records[outerIdx][innerIdx].type <= Type::Octuple && !this->records[outerIdx][innerIdx].cards.empty())
							{
								if (this->records[outerIdx][innerIdx].player == this->dealer)
									isAntiSpring = false;
								else
									isSpring = false;
							}
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
					this->amounts = std::vector<Amount>(4);
					for (Player player = 0; player < 4; ++player)
						this->amounts[player] = base * playerFlags[player];
#if ((defined _MSVC_LANG && _MSVC_LANG >= 201703L) || (!defined _MSVC_LANG && defined __cplusplus && __cplusplus >= 201103L))
					[[fallthrough]];
#endif
				}
				case 4:
					return true;
				default:
					return false;
				}
			else
				return false;
		}
		else
			return false;
	}
	bool isAbsolutelyLargest(const Hand& hand) const override final
	{
		return Type::QuadrupleJokers == hand.type;
	}
	std::string getBasisString() const override final
	{
		return this->amounts.size() == 1 ? "当前倍数：" + std::to_string(this->amounts[0]) + "\n" : "";
	}
	std::string getPreRoundString() const override final
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
			if (0 == callerAndRobberCount && this->records[0].size() == 4)
				return "无人叫地主，强制玩家 " + std::to_string(this->records[0][0].player + 1) + " 为地主。";
			else
			{
				std::string preRoundString{};
				for (const Hand& hand : this->records[0])
				{
					const std::string playerString = std::to_string(hand.player + 1);
					switch (hand.cards.size())
					{
					case 0:
						preRoundString += "不叫（玩家 " + playerString + "） -> ";
						break;
					case 1:
						switch (hand.cards[0].point)
						{
						case 0:
							preRoundString += "不叫（玩家 " + playerString + "） -> ";
							break;
						case 1:
						case 2:
						case 3:
							preRoundString += std::to_string(hand.cards[0].point) + "分（玩家 " + playerString + "） -> ";
							break;
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
	Landlords4P() : Poker()
	{
		this->name = "四人斗地主";
	}
	bool initialize() override final
	{
		if (this->status >= Status::Ready)
		{
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->values.set(JOKER_POINT, value++);
			this->players = std::vector<std::vector<Card>>(4);
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
	bool deal() override final
	{
		const size_t playerCount = this->players.size();
		if (this->status >= Status::Initialized && this->checkPlayerCount(playerCount))
		{
			this->deck.clear();
			this->add54CardsToDeck();
			this->add54CardsToDeck(); // We confirmed that this line is not mis-duplicated. 
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < playerCount; ++player)
			{
				this->players[player] = std::vector<Card>(Landlords4P::CardCountPerPlayer);
				for (size_t idx = 0; idx < Landlords4P::CardCountPerPlayer; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records = std::vector<std::vector<Hand>>{ std::vector<Hand>{} };
			std::uniform_int_distribution<size_t> distribution(0, playerCount - 1);
			this->currentPlayer = (Player)(distribution(this->seed));
			this->dealer = INVALID_PLAYER;
			this->lastHand = Hand{};
			this->amounts.clear();
			this->status = Status::Dealt;
			return true;
		}
		else
			return false;
	}
	bool setLandlord(const Score score) override final
	{
		const Point point = static_cast<Point>(score);
		if (Status::Dealt == this->status && this->records.size() == 1 && /* 0 <= this->currentPlayer && */this->currentPlayer < this->players.size())
			switch (this->records[0].size())
			{
			case 0:
				switch (score)
				{
				case Score::None:
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{} });
					this->nextPlayer();
					return true;
				case Score::One:
				case Score::Two:
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{ Card{ point } } });
					this->nextPlayer();
					this->lastHand = this->records[0][0];
					return true;
				case Score::Three:
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{ Card{ point } } });
					this->dealer = this->currentPlayer;
					this->lastHand = Hand{};
					this->amounts = std::vector<Amount>{ 3 };
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
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{} });
					this->nextPlayer();
					return true;
				case Score::One:
				case Score::Two:
					if (!this->lastHand || (this->lastHand.cards.size() == 1 && point > this->lastHand.cards[0].point))
					{
						this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{ Card{ point } } });
						this->nextPlayer();
						this->lastHand = this->records[0].back();
						return true;
					}
					else
						return false;
				case Score::Three:
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{ Card{ point } } });
					this->dealer = this->currentPlayer;
					this->lastHand = Hand{};
					this->amounts = std::vector<Amount>{ 3 };
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
							this->amounts = std::vector<Amount>{ this->lastHand.cards[0].point };
						}
						else
							return false;
					}
					else
						this->currentPlayer = this->records[0][0].player;
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{} });
					break;
				case Score::One:
				case Score::Two:
					if (!this->lastHand || (this->lastHand.cards.size() == 1 && point > this->lastHand.cards[0].point))
					{
						this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{ Card{ point } } });
						this->amounts = std::vector<Amount>{ this->lastHand.cards[0].point };
						break;
					}
					else
						return false;
				case Score::Three:
					this->records[0].push_back(Hand{ this->currentPlayer, std::vector<Card>{ Card{ point } } });
					this->amounts = std::vector<Amount>{ 3 };
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
	bool display(const std::vector<Player>& selectedPlayers) const override final
	{
		return this->status >= Status::Assigned ? Poker::display(selectedPlayers, "地主", "地主牌：" + this->cards2string(this->deck, "", " | ", "（已公开）", "（空）") + "\n\n") : Poker::display(selectedPlayers, "拥有明牌", "地主牌：" + this->cards2string(this->deck, "", " | ", "（未公开）", "（空）") + "\n\n");
	}
};

class BigTwo : public Poker /* Previous: Landlords4P | Next: ThreeTwoOne */
{
public:
	static const size_t MinimumPlayerCount = 4, MaximumPlayerCount = 4, CardCountPerPlayer = 13;
	
private:
	bool checkPlayerCount(const size_t playerCount) const override final
	{
		return BigTwo::MinimumPlayerCount <= playerCount && playerCount <= BigTwo::MaximumPlayerCount;
	}
	bool isRealHand(const Hand& hand) const override final
	{
		switch (hand.type)
		{
		case Type::Single: // 单牌
		case Type::SingleStraight: // 顺子（长度只能为 5）：可被一条龙|同花顺、金刚、葫芦/俘虏/乌龙/副路、同花以及比自己大的顺子盖过
		case Type::SingleFlush: // 同花（长度只能为 5）：可被一条龙|同花顺、金刚、葫芦/俘虏/乌龙/副路以及比自己大的同花盖过
		case Type::SingleFlushStraight: // 一条龙|同花顺（长度只能为 5）
		case Type::Pair: // 对子
		case Type::Triple: // 三条
		case Type::TripleWithPair: // 葫芦/俘虏/乌龙/副路：可被一条龙|同花顺、金刚、以及比自己大的葫芦/俘虏/乌龙/副路盖过
		case Type::QuadrupleWithSingle: // 金刚：可被一条龙|同花顺和比自己大的金刚盖过
			return hand.player != INVALID_PLAYER && !hand.cards.empty();
		case Type::Empty:
		case Type::PairStraight:
		case Type::PairStraightWithSingle:
		case Type::PairJokers:
		case Type::TripleWithSingle:
		case Type::TripleWithPairSingle:
		case Type::TripleStraight:
		case Type::TripleStraightWithSingle:
		case Type::TripleStraightWithSingles:
		case Type::TripleStraightWithPairs:
		case Type::Quadruple:
		case Type::QuadrupleWithSingleSingle:
		case Type::QuadrupleWithPairPair:
		case Type::QuadrupleStraight:
		case Type::QuadrupleStraightWithSingle:
		case Type::QuadrupleJokers:
		case Type::Quintuple:
		case Type::Sextuple:
		case Type::Septuple:
		case Type::Octuple:
		case Type::Invalid:
		default:
			return false;
		}
	}
	bool coverLastHand(const Hand& currentHand) const override final
	{
		if (this->isRealHand(this->lastHand) && this->isRealHand(currentHand))
			switch (this->lastHand.type)
			{
			case Type::Single: // 单牌
			case Type::SingleFlushStraight: // 一条龙|同花顺（长度只能为 5）
			case Type::Pair: // 对子
			case Type::Triple: // 三条
				return currentHand.type == this->lastHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit));
			case Type::SingleStraight: // 顺子（长度只能为 5）：可被一条龙|同花顺、金刚、葫芦/俘虏/乌龙/副路、同花以及比自己大的顺子盖过
				return Type::SingleFlushStraight == currentHand.type || Type::QuadrupleWithSingle == currentHand.type || Type::TripleWithPair == currentHand.type || Type::SingleFlush == currentHand.type || (Type::SingleStraight == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::SingleFlush: // 同花（长度只能为 5）：可被一条龙|同花顺、金刚、葫芦/俘虏/乌龙/副路以及比自己大的同花盖过
				return Type::SingleFlushStraight == currentHand.type || Type::QuadrupleWithSingle == currentHand.type || Type::TripleWithPair == currentHand.type || (Type::SingleFlush == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::TripleWithPair: // 葫芦/俘虏/乌龙/副路：可被一条龙|同花顺、金刚、以及比自己大的葫芦/俘虏/乌龙/副路盖过
				return Type::SingleFlushStraight == currentHand.type || Type::QuadrupleWithSingle == currentHand.type || (Type::TripleWithPair == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::QuadrupleWithSingle: // 金刚：可被一条龙|同花顺和比自己大的金刚盖过
				return Type::SingleFlushStraight == currentHand.type || (Type::QuadrupleWithSingle == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::Empty:
			case Type::PairStraight:
			case Type::PairStraightWithSingle:
			case Type::PairJokers:
			case Type::TripleWithSingle:
			case Type::TripleWithPairSingle:
			case Type::TripleStraight:
			case Type::TripleStraightWithSingle:
			case Type::TripleStraightWithSingles:
			case Type::TripleStraightWithPairs:
			case Type::Quadruple:
			case Type::QuadrupleWithSingleSingle:
			case Type::QuadrupleWithPairPair:
			case Type::QuadrupleStraight:
			case Type::QuadrupleStraightWithSingle:
			case Type::QuadrupleJokers:
			case Type::Quintuple:
			case Type::Sextuple:
			case Type::Septuple:
			case Type::Octuple:
			case Type::Invalid:
			default:
				return false;
			}
		else
			return false;
	}
	bool processHand(Hand& hand, std::vector<Candidate>& candidates) const override final
	{
		hand.type = Type::Invalid;
		candidates.clear();
		std::vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (this->values[card.point])
				++counts[card.point];
			else
				return false;
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && (valueA > valueB || (valueA == valueB && a.suit > b.suit))); });
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
	bool computeAmounts() override final
	{
		if (Status::Over == this->status)
			switch (this->amounts.size())
			{
			case 0:
			{
				if (this->players.size() != 4)
					return false;
				Count winnerCount = 0;
				this->amounts = std::vector<Amount>(4);
				for (size_t idx = 0; idx < 4; ++idx)
				{
					const size_t n = this->players[idx].size();
					if (n <= 0)
						++winnerCount;
					else if (n < 8)
						this->amounts[idx] = static_cast<Amount>(n);
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
#if ((defined _MSVC_LANG && _MSVC_LANG >= 201703L) || (!defined _MSVC_LANG && defined __cplusplus && __cplusplus >= 201103L))
				[[fallthrough]];
#endif
			}
			case 4:
				return true;
			default:
				return false;
			}
		else
			return false;
	}
	bool isAbsolutelyLargest(const Hand& hand) const override final
	{
		return (Type::Single == hand.type || Type::Pair == hand.type || Type::Triple == hand.type || Type::Quadruple == hand.type || Type::SingleFlushStraight == hand.type) && (!hand.cards.empty() && Card { 2, Suit::Spade } == hand.cards[0]);
	}
	std::string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
			return this->records[0].back().cards.size() == 1 && 1 == this->values[this->records[0].back().cards[0].point] && Suit::Diamond == this->records[0].back().cards[0].suit ? "玩家 " + std::to_string(this->records[0].back().player + 1) + " 拥有最小的牌（" + (std::string)this->records[0].back().cards[0] + "），拥有发牌权。" : "预备回合信息检验异常。";
	}
	
public:
	BigTwo() : Poker()
	{
		this->name = "锄大地";
	}
	bool initialize() override final
	{
		if (this->status >= Status::Ready)
		{
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->players = std::vector<std::vector<Card>>(4);
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
	bool deal() override final
	{
		const size_t playerCount = this->players.size();
		if (this->status >= Status::Initialized && this->checkPlayerCount(playerCount))
		{
			this->deck.clear();
			this->add52CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < playerCount; ++player)
			{
				this->players[player] = std::vector<Card>(BigTwo::CardCountPerPlayer);
				for (size_t idx = 0; idx < BigTwo::CardCountPerPlayer; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records = std::vector<std::vector<Hand>>{ std::vector<Hand>{} };
			for (Player player = 0; player < playerCount; ++player)
				this->records[0].push_back(Hand{ player, std::vector<Card>{ this->players[player].back() } });
			sort(this->records[0].begin(), this->records[0].end(), [this](Hand a, Hand b) { const Value valueA = this->values[a.cards.back().point], valueB = this->values[b.cards.back().point]; return valueA > valueB || (valueA == valueB && a.cards.back().suit > b.cards.back().suit); });
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
	bool display(const std::vector<Player>& selectedPlayers) const override final
	{
		return Poker::display(selectedPlayers, "方块3 先出", "");
	}
};

class ThreeTwoOne : public Poker /* Previous: BigTwo | Next: Wuguapi */
{
public:
	static const size_t MinimumPlayerCount = 4, MaximumPlayerCount = 4, CardCountPerPlayer = 13;
	
private:
	bool checkPlayerCount(const size_t playerCount) const override final
	{
		return ThreeTwoOne::MinimumPlayerCount <= playerCount && playerCount <= ThreeTwoOne::MaximumPlayerCount;
	}
	bool isRealHand(const Hand& hand) const override final
	{
		switch (hand.type)
		{
		case Type::Single: // 单牌
		case Type::SingleStraight: // 顺子
		case Type::Pair: // 对子
		case Type::PairStraight: // 连对
		case Type::PairStraightWithSingle: // 连对夹一
		case Type::Triple: // 三张
		case Type::TripleWithPair: // 三两不一
		case Type::TripleWithPairSingle: // 三两一
		case Type::TripleStraight: // 三顺
		case Type::TripleStraightWithSingle: // 三顺夹一
		case Type::Quadruple: // 四张
		case Type::QuadrupleWithSingle: // 四夹一
		case Type::QuadrupleStraight: // 四顺
		case Type::QuadrupleStraightWithSingle: // 四顺夹一
			return !hand.cards.empty();
		case Type::Empty:
		case Type::SingleFlush:
		case Type::SingleFlushStraight:
		case Type::PairJokers:
		case Type::TripleWithSingle:
		case Type::TripleStraightWithSingles:
		case Type::TripleStraightWithPairs:
		case Type::QuadrupleWithSingleSingle:
		case Type::QuadrupleWithPairPair:
		case Type::QuadrupleJokers:
		case Type::Quintuple:
		case Type::Sextuple:
		case Type::Septuple:
		case Type::Octuple:
		case Type::Invalid:
		default:
			return false;
		}
	}
	bool coverLastHand(const Hand& currentHand) const override final
	{
		return this->isRealHand(this->lastHand) && this->isRealHand(currentHand) && currentHand.type == this->lastHand.type && currentHand.cards.size() == this->lastHand.cards.size() && this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point];
	}
	bool processHand(Hand& hand, std::vector<Candidate>& candidates) const override final
	{
		hand.type = Type::Invalid;
		std::vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (this->values[card.point])
				++counts[card.point];
			else
			{
				candidates.clear();
				return false;
			}
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && (valueA > valueB || (valueA == valueB && a.suit > b.suit))); });
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
					std::vector<Candidate> potentialHands{};
					const std::string cardString0 = this->point2description(hand.cards[0].point), cardString3 = this->point2description(hand.cards[3].point);
					if (this->values[hand.cards[3].point] + 1 == this->values[hand.cards[0].point])
					{
						potentialHands.emplace_back(hand, "解析为长度为 2 且点数为 " + cardString0 + " 的三顺（``Type::TripleStraight``）");
						potentialHands.back().hand.type = Type::TripleStraight; // 三顺
					}
					potentialHands.emplace_back(hand, "解析为三条 " + cardString0 + " 的三两一，两为一对 " + cardString3 + "，一为一张 " + cardString3 + "（``Type::TripleWithPairSingle``）");
					potentialHands.back().hand.type = Type::TripleWithPairSingle; // 三两一
					rotate(hand.cards.begin(), hand.cards.begin() + 3, hand.cards.end());
					potentialHands.emplace_back(hand, "解析为三条 " + cardString3 + " 的三两一，两为一对 " + cardString0 + "，一为一张 " + cardString0 + "（``Type::TripleWithPairSingle``）");
					potentialHands.back().hand.type = Type::TripleWithPairSingle; // 三两一
					if (3 == hand.cards[0].point && 2 == hand.cards[3].point)
					{
						potentialHands.emplace_back(hand, "解析为长度为 2 且点数为 3 的三顺（``Type::TripleStraight``）");
						potentialHands.back().hand.type = Type::TripleStraight; // 三顺
					}
					if (this->lastHand && hand.player != this->lastHand.player)
						for (std::vector<Candidate>::iterator it = potentialHands.begin(); it != potentialHands.end();)
						{
							if (this->coverLastHand(it->hand))
								++it;
							else
								it = potentialHands.erase(it);
						}
					switch (potentialHands.size())
					{
					case 0:
						candidates.clear();
						return false;
					case 1:
						hand = std::move(potentialHands[0].hand);
						candidates.clear();
						return true;
					default:
						if (candidates.size() == 1)
						{
							const std::vector<Candidate>::iterator it = find_if(potentialHands.begin(), potentialHands.end(), [&candidates](const Candidate& candidate) { return candidate.hand == candidates[0].hand; });
							if (it != potentialHands.end())
							{
								hand = std::move(it->hand);
								candidates.clear();
								return true;
							}
						}
						candidates = std::move(potentialHands);
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
						std::vector<Card> bodyCards(hand.cards);
						bodyCards.erase(bodyCards.begin() + 3);
						if (this->judgeStraight(bodyCards, 3, 3, true))
						{
							bodyCards.push_back(hand.cards[3]);
							hand.cards = std::move(bodyCards);
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
						std::vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
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
							std::vector<Card> bodyCards(hand.cards);
							bodyCards.erase(bodyCards.begin() + 2);
							if (this->judgeStraight(bodyCards, 2, 3, true))
							{
								bodyCards.push_back(hand.cards[2]);
								hand.cards = std::move(bodyCards);
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
						std::vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
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
						std::vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (this->judgeStraight(bodyCards, 4, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = std::move(bodyCards);
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
						else
							return false;
					case 2:
						if (2 == counts[2] && 2 == counts[3])
						{
							std::vector<Card> bodyCards(hand.cards);
							bodyCards.erase(bodyCards.begin() + 2);
							if (this->judgeStraight(bodyCards, 2, 3, true))
							{
								bodyCards.push_back(hand.cards[2]);
								hand.cards = std::move(bodyCards);
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
						std::vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (this->judgeStraight(bodyCards, 2, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = std::move(bodyCards);
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
						std::vector<Card> bodyCards(hand.cards);
						bodyCards.erase(bodyCards.begin() + 3);
						if (this->judgeStraight(bodyCards, 3, 3, true))
						{
							bodyCards.push_back(hand.cards[3]);
							hand.cards = std::move(bodyCards);
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
						std::vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (this->judgeStraight(bodyCards, 3, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = std::move(bodyCards);
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
						std::vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (this->judgeStraight(bodyCards, 2, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = std::move(bodyCards);
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
						std::vector<Card> bodyCards(hand.cards);
						bodyCards.erase(bodyCards.begin() + 2);
						if (this->judgeStraight(bodyCards, 2, 3, true))
						{
							bodyCards.push_back(hand.cards[2]);
							hand.cards = std::move(bodyCards);
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
						std::vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (this->judgeStraight(bodyCards, 2, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = std::move(bodyCards);
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
						std::vector<Card> bodyCards(hand.cards);
						bodyCards.erase(bodyCards.begin() + 3);
						if (this->judgeStraight(bodyCards, 3, 3, true))
						{
							bodyCards.push_back(hand.cards[3]);
							hand.cards = std::move(bodyCards);
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
							std::vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
							if (this->judgeStraight(bodyCards, 3, 3, true))
							{
								bodyCards.push_back(hand.cards.back());
								hand.cards = std::move(bodyCards);
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
							std::vector<Card> bodyCards(hand.cards);
							bodyCards.erase(bodyCards.begin() + 2);
							if (this->judgeStraight(bodyCards, 2, 3, true))
							{
								bodyCards.push_back(hand.cards[2]);
								hand.cards = std::move(bodyCards);
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
						std::vector<Card> bodyCards(hand.cards.begin(), hand.cards.end() - 1);
						if (2 == this->judgeStraight(bodyCards, 3, true))
						{
							bodyCards.push_back(hand.cards.back());
							hand.cards = std::move(bodyCards);
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
	bool computeAmounts() override final
	{
		if (Status::Over == this->status)
			switch (this->amounts.size())
			{
			case 0:
			{
				if (this->players.size() != 4)
					return false;
				Player winner = INVALID_PLAYER;
				this->amounts = std::vector<Amount>(4);
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
#if ((defined _MSVC_LANG && _MSVC_LANG >= 201703L) || (!defined _MSVC_LANG && defined __cplusplus && __cplusplus >= 201103L))
				[[fallthrough]];
#endif
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
	bool isAbsolutelyLargest(const Hand& hand) const override final
	{
		return Type::Single <= hand.type && hand.type <= Type::QuadrupleStraightWithSingle && !hand.cards.empty() && 2 == hand.cards[0].point;
	}
	std::string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else if (this->records[0].back().cards.size() == 1 && 1 == this->values[this->records[0].back().cards[0].point] && Suit::Diamond == this->records[0].back().cards[0].suit)
			return "玩家 " + std::to_string(this->records[0].back().player + 1) + " 拥有最小的牌（" + (std::string)this->records[0].back().cards[0] + "），拥有发牌权。";
		else
			return "预备回合信息检验异常。";
	}
	
public:
	ThreeTwoOne() : Poker()
	{
		this->name = "三两一";
	}
	bool initialize() override final
	{
		if (this->status >= Status::Ready)
		{
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->players = std::vector<std::vector<Card>>(4);
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
	bool deal() override final
	{
		const size_t playerCount = this->players.size();
		if (this->status >= Status::Initialized && this->checkPlayerCount(playerCount))
		{
			this->deck.clear();
			this->add52CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < playerCount; ++player)
			{
				this->players[player] = std::vector<Card>(ThreeTwoOne::CardCountPerPlayer);
				for (size_t idx = 0; idx < ThreeTwoOne::CardCountPerPlayer; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records = std::vector<std::vector<Hand>>{ std::vector<Hand>{} };
			for (Player player = 0; player < playerCount; ++player)
				this->records[0].push_back(Hand{ player, std::vector<Card>{ this->players[player].back() } });
			sort(this->records[0].begin(), this->records[0].end(), [this](Hand a, Hand b) { const Value valueA = this->values[a.cards.back().point], valueB = this->values[b.cards.back().point]; return valueA > valueB || (valueA == valueB && a.cards.back().suit > b.cards.back().suit); });
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
	bool nextPlayer() override final
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
				offsetPlayer = static_cast<Player>((offsetPlayer + 1) % 4);
				if (flags[offsetPlayer])
				{
					this->currentPlayer = offsetPlayer;
					return true;
				}
			}
			return false;
		}
	}
	bool display(const std::vector<Player>& selectedPlayers) const override final
	{
		return Poker::display(selectedPlayers, "方块3 先出", "");
	}
};

class Wuguapi : public Poker /* Previous: ThreeTwoOne | Next: Qiguiwuersan */
{
public:
	static const size_t MinimumPlayerCount = 2, MaximumPlayerCount = 10, CardCountPerPlayer = 5;
	
private:
	bool checkPlayerCount(const size_t playerCount) const override final
	{
		return Wuguapi::MinimumPlayerCount <= playerCount && playerCount <= Wuguapi::MaximumPlayerCount;
	}
	bool nextPlayer() override final
	{
		const size_t playerCount = this->players.size();
		if (this->checkPlayerCount(playerCount))
			for (Count count = 1; count < playerCount; ++count)
			{
				if (++this->currentPlayer >= playerCount)
					this->currentPlayer = 0;
				if (!this->players[this->currentPlayer].empty())
					return true;
			}
		return false;
	}
	bool isRealHand(const Hand& hand) const override final
	{
		switch (hand.type)
		{
		case Type::Single: // 单牌
		case Type::SingleStraight: // 顺子
		case Type::SingleFlush: // 同花
		case Type::SingleFlushStraight: // 一条龙|同花顺
		case Type::Pair: // 对子
		case Type::PairJokers: // 对鬼
		case Type::Triple: // 三条
		case Type::TripleWithPair: // 葫芦/俘虏/乌龙/副路
		case Type::Quadruple: // 四条
		case Type::QuadrupleWithSingle: // 金刚
			return true;
		case Type::Empty:
		case Type::PairStraight:
		case Type::PairStraightWithSingle:
		case Type::TripleWithSingle:
		case Type::TripleWithPairSingle:
		case Type::TripleStraight:
		case Type::TripleStraightWithSingle:
		case Type::TripleStraightWithSingles:
		case Type::TripleStraightWithPairs:
		case Type::QuadrupleWithSingleSingle:
		case Type::QuadrupleWithPairPair:
		case Type::QuadrupleStraight:
		case Type::QuadrupleStraightWithSingle:
		case Type::QuadrupleJokers:
		case Type::Quintuple:
		case Type::Sextuple:
		case Type::Septuple:
		case Type::Octuple:
		case Type::Invalid:
		default:
			return false;
		}
	}
	bool coverLastHand(const Hand& currentHand) const override final
	{
		if (this->isRealHand(this->lastHand) && this->isRealHand(currentHand))
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
				case 5: // 可被一条龙|同花顺、金刚、葫芦/俘虏/乌龙/副路、同花以及比自己大的顺子盖过
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
				case 5: // 可被一条龙|同花顺、金刚、葫芦/俘虏/乌龙/副路、以及比自己大的同花盖过
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
			case Type::TripleWithPair: // 葫芦/俘虏/乌龙/副路：可被一条龙|同花顺、金刚、以及比自己大的葫芦/俘虏/乌龙/副路盖过
				return Type::SingleFlushStraight == currentHand.type || Type::QuadrupleWithSingle == currentHand.type || (Type::TripleWithPair == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::Quadruple: // 四条：可被一条龙|同花顺和比自己大的四条盖过
				return (Type::SingleFlushStraight == currentHand.type && currentHand.cards.size() == 4) || (Type::Quadruple == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::QuadrupleWithSingle: // 金刚：可被一条龙|同花顺和比自己大的金刚盖过
				return Type::SingleFlushStraight == currentHand.type || (Type::QuadrupleWithSingle == currentHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit)));
			case Type::Empty:
			case Type::PairStraight:
			case Type::PairStraightWithSingle:
			case Type::TripleWithSingle:
			case Type::TripleWithPairSingle:
			case Type::TripleStraight:
			case Type::TripleStraightWithSingle:
			case Type::TripleStraightWithSingles:
			case Type::TripleStraightWithPairs:
			case Type::QuadrupleWithSingleSingle:
			case Type::QuadrupleWithPairPair:
			case Type::QuadrupleStraight:
			case Type::QuadrupleStraightWithSingle:
			case Type::QuadrupleJokers:
			case Type::Quintuple:
			case Type::Sextuple:
			case Type::Septuple:
			case Type::Octuple:
			case Type::Invalid:
			default:
				return false;
			}
		else
			return false;
	}
	bool processHand(Hand& hand, std::vector<Candidate>& candidates) const override final
	{
		hand.type = Type::Invalid;
		candidates.clear();
		bool blackJoker = false, redJoker = false;
		std::vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (JOKER_POINT == card.point)
				switch (card.suit)
				{
				case Suit::Black:
					if (blackJoker)
						return false;
					else
					{
						blackJoker = true;
						++counts[JOKER_POINT];
						break;
					}
				case Suit::Red:
					if (redJoker)
						return false;
					else
					{
						redJoker = true;
						++counts[JOKER_POINT];
						break;
					}
				case Suit::Diamond:
				case Suit::Club:
				case Suit::Heart:
				case Suit::Spade:
				case Suit::Cover:
				default:
					return false;
				}
			else if (this->values[card.point])
				++counts[card.point];
			else
				return false;
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && (valueA > valueB || (valueA == valueB && a.suit > b.suit))); });
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
	bool isAbsolutelyLargest(const Hand& hand) const override final
	{
		return ((Type::Single == hand.type || Type::PairJokers == hand.type) && Card { JOKER_POINT, Suit::Red } == hand.cards[0]) || (Type::SingleFlushStraight == hand.type && Card{ 5, Suit::Spade } == hand.cards[0]);
	}
	bool processBasis(const Hand& hand) override final
	{
		if (Status::Started == this->status && !this->records.empty() && !this->records.back().empty())
		{
			const size_t playerCount = this->players.size();
			if (this->amounts.size() == playerCount && this->checkPlayerCount(playerCount))
				if (this->isRealHand(hand))
				{
					/* Compute the winner bonus */
					if (this->deck.empty() && this->players[hand.player].empty())
					{
						Count winnerCount = 0;
						for (const Amount& amount : this->amounts)
							if (amount >> 8)
								++winnerCount;
						if (winnerCount >= playerCount)
							return false;
						this->amounts[hand.player] += static_cast<Amount>(winnerCount) << 8;
					}
					
					/* Compute the winner integrals */
					if (this->isAbsolutelyLargest(hand))
					{
						/* Scan the integrals */
						Integral integral = 0;
						Count emptyCount = 1;
						for (std::vector<Hand>::const_reverse_iterator it = this->records.back().rbegin() + 1; it != this->records.back().rend(); ++it)
							if (this->isRealHand(*it))
								if (this->isAbsolutelyLargest(*it))
									break;
								else
								{
									for (const Card& card : it->cards)
										switch (card.point)
										{
										case 5:
											integral += 5;
											break;
										case 10:
										case 13:
											integral += 10;
											break;
										default:
											break;
										}
									emptyCount = 1;
								}
							else if (Type::Empty == it->type && it->cards.empty())
							{
								if (++emptyCount >= playerCount)
								{
									if (this->records.back().rend() == ++it || !this->isRealHand(*it))
										return false;
									else
										break;
								}
							}
							else
								return false;
						
						/* Apply the amount */
						Player bonusPlayer = hand.player;
						this->amounts[bonusPlayer] += integral;
						
						/* Draw */
						for (Count count = 0; count < playerCount; ++count)
						{
							while (this->players[bonusPlayer].size() < Wuguapi::CardCountPerPlayer)
								if (this->deck.empty())
									return true;
								else
								{
									this->players[bonusPlayer].emplace_back(std::move(this->deck.back()));
									this->deck.pop_back();
								}
							this->sortCards(this->players[bonusPlayer]);
							if (++bonusPlayer >= playerCount)
								bonusPlayer = 0;
						}
					}
					return true;
				}
				else if (Type::Empty == hand.type && hand.cards.empty()) // Compute the winner integrals
				{
					/* Judge an end of a circle */
					std::vector<Hand>::const_reverse_iterator it = this->records.back().rbegin();
					for (Count emptyCount = 2; emptyCount < playerCount;)
						if (this->records.back().rend() == ++it || this->isRealHand(*it))
							return true;
						else if (Type::Empty == it->type && it->cards.empty())
							++emptyCount;
						else
							return false;
					if (this->records.back().rend() == ++it || !this->isRealHand(*it))
						return false;
					
					/* Scan the integrals */
					Integral integral = 0;
					Player bonusPlayer = it->player;
					for (const Card& card : it->cards)
						switch (card.point)
						{
						case 5:
							integral += 5;
							break;
						case 10:
						case 13:
							integral += 10;
							break;
						default:
							break;
						}
					for (Count emptyCount = 1; ++it != this->records.back().rend();)
						if (this->isRealHand(*it))
							if (this->isAbsolutelyLargest(*it))
								break;
							else
							{
								for (const Card& card : it->cards)
									switch (card.point)
									{
									case 5:
										integral += 5;
										break;
									case 10:
									case 13:
										integral += 10;
										break;
									default:
										break;
									}
								emptyCount = 1;
							}
						else if (Type::Empty == it->type && it->cards.empty())
						{
							if (++emptyCount >= playerCount)
							{
								if (this->records.back().rend() == ++it || !this->isRealHand(*it))
									return false;
								else
									break;
							}
						}
						else
							return false;
					
					/* Apply the amount */
					this->amounts[bonusPlayer] += integral;
					
					/* Draw */
					for (Count count = 0; count < playerCount; ++count)
					{
						while (this->players[bonusPlayer].size() < Wuguapi::CardCountPerPlayer)
							if (this->deck.empty())
								return true;
							else
							{
								this->players[bonusPlayer].emplace_back(std::move(this->deck.back()));
								this->deck.pop_back();
							}
						this->sortCards(this->players[bonusPlayer]);
						if (++bonusPlayer >= playerCount)
							bonusPlayer = 0;
					}
					return true;
				}
				else
					return false;
			else
				return false;
		}
		else
			return false;
	}
	bool isOver() const override final
	{
		if (this->status >= Status::Started && this->deck.empty())
		{
			bool hasCards = false;
			for (const std::vector<Card>& cards : this->players)
				if (!cards.empty())
				{
					if (hasCards)
						return false;
					else
						hasCards = true;
				}
			return true;
		}
		return false;
	}
	std::string getBasisString() const override final
	{
		const size_t playerCount = this->players.size();
		if (playerCount == this->amounts.size() && this->checkPlayerCount(playerCount))
		{
			std::string basisString = "积分信息：";
			bool flag = false;
			for (Player player = 0; player < playerCount; ++player)
			{
				Amount upperAmount = this->amounts[player] >> 8, lowerAmount = this->amounts[player] & 0xFF/* 0b11111111*/;
				if (upperAmount)
				{
					if (flag)
						basisString += "；";
					basisString += "玩家 " + std::to_string(player + 1) + " 是第 " + std::to_string(upperAmount) + " 位出完牌的玩家";
					if (lowerAmount)
						basisString += "，得 " + std::to_string(lowerAmount) + " 积分";
					flag = true;
				}
				else if (lowerAmount)
				{
					if (flag)
						basisString += "；";
					basisString += "玩家 " + std::to_string(player + 1) + " 得 " + std::to_string(lowerAmount) + " 积分";
					flag = true;
				}
			}
			basisString += flag ? "。\n" : "暂无玩家获得积分或出完牌。\n";
			return basisString;
		}
		else
			return "";
	}
	std::string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			std::string preRoundString{};
			for (const Hand& hand : this->records[0])
				if (hand.cards.size() == 1)
					preRoundString += (std::string)hand.cards[0] + "（玩家 " + std::to_string(hand.player + 1) + "） > ";
				else
					return "预备回合信息检验异常。";
			preRoundString.erase(preRoundString.length() - 3, 3);
			return preRoundString;
		}
	}
	std::string getAmountString() const override final
	{
		const size_t playerCount = this->players.size();
		if (Status::Over == this->status && playerCount == this->amounts.size() && this->checkPlayerCount(playerCount))
		{
			/* Compress rankings */
			std::vector<Player> sortedPlayers(playerCount);
			std::vector<Amount> upperAmounts(playerCount), lowerAmounts(playerCount);
			for (Player player = 0; player < playerCount; ++player)
			{
				sortedPlayers[player] = player;
				upperAmounts[player] = this->amounts[player] >> 8;
				lowerAmounts[player] = this->amounts[player] & 0xFF/* 0b11111111*/;
			}
			sort(sortedPlayers.begin(), sortedPlayers.end(), [&lowerAmounts](const Player playerA, const Player playerB) {return lowerAmounts[playerA] > lowerAmounts[playerB]; });
			std::vector<Ranking> rankings(playerCount);
			rankings[sortedPlayers[0]] = 1;
			Ranking zippedRanking = 1, ranking = 1;
			for (size_t idx = 1; idx < playerCount; ++idx)
			{
				if (lowerAmounts[sortedPlayers[idx - 1]] == lowerAmounts[sortedPlayers[idx]])
					++zippedRanking;
				else
				{
					ranking += zippedRanking;
					zippedRanking = 1;
				}
				rankings[sortedPlayers[idx]] = ranking;
			}
			
			/* Output rankings */
			std::string amountString = "/* 结算信息 */\n";
			bool flag = false;
			for (Player player = 0; player < playerCount; ++player)
				if (upperAmounts[player])
				{
					if (flag)
						amountString += "；";
					amountString += "玩家 " + std::to_string(player + 1) + " 是第 " + std::to_string(upperAmounts[player]) + " 位出完牌的玩家";
					if (lowerAmounts[player])
						amountString += "，得 " + std::to_string(lowerAmounts[player]) + " 积分，积分排名为 " + std::to_string(rankings[player]);
					flag = true;
				}
				else if (lowerAmounts[player])
				{
					if (flag)
						amountString += "；";
					amountString += "玩家 " + std::to_string(player + 1) + " 得 " + std::to_string(lowerAmounts[player]) + " 积分，积分排名为 " + std::to_string(rankings[player]);
					flag = true;
				}
			amountString += flag ? "。\n" : "结算信息异常，请各位玩家自行计算结算信息。\n";
			return amountString;
		}
		else
			return "结算信息异常，请各位玩家自行计算结算信息。\n";
	}
	
public:
	Wuguapi() : Poker()
	{
		this->name = "五瓜皮";
	}
	bool initialize() override final { return this->initialize(Wuguapi::MinimumPlayerCount); }
	bool initialize(const size_t playerCount) override final
	{
		if (this->status >= Status::Ready && this->checkPlayerCount(playerCount))
		{
			Value value = 1;
			for (Point point = 6; point <= 13; ++point)
				this->values.set(point, value++);
			for (Point point = 1; point <= 5; ++point)
				this->values.set(point, value++);
			this->values.set(JOKER_POINT, value++);
			this->players = std::vector<std::vector<Card>>(playerCount);
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
	bool deal() override final
	{
		const size_t playerCount = this->players.size();
		if (this->status >= Status::Initialized && this->checkPlayerCount(playerCount))
		{
			this->deck.clear();
			this->add54CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < playerCount; ++player)
			{
				this->players[player] = std::vector<Card>(Wuguapi::CardCountPerPlayer);
				for (size_t idx = 0; idx < Wuguapi::CardCountPerPlayer; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records = std::vector<std::vector<Hand>>{ std::vector<Hand>{} };
			for (Player player = 0; player < playerCount; ++player)
				this->records[0].push_back(Hand{ player, std::vector<Card>{ this->players[player].back() } });
			sort(this->records[0].begin(), this->records[0].end(), [this](Hand a, Hand b) { const Value valueA = this->values[a.cards.back().point], valueB = this->values[b.cards.back().point]; return valueA > valueB || (valueA == valueB && a.cards.back().suit > b.cards.back().suit); });
			this->currentPlayer = this->records[0].back().player;
			this->dealer = this->records[0].back().player;
			this->lastHand = Hand{};
			this->amounts = std::vector<Amount>(playerCount);
			this->status = Status::Assigned;
			return true;
		}
		else
			return false;
	}
	bool display(const std::vector<Player>& selectedPlayers) const override final
	{
		return Poker::display(selectedPlayers, "最小先出", "牌堆（自下往上）：" + this->cards2string(this->deck, "", " | ", "", "（空）") + "\n\n");
	}
};

class Qiguiwuersan : public Poker /* Previous: Wuguapi | Next: Qiguiwueryi */
{
public:
	static const size_t MinimumPlayerCount = 2, MaximumPlayerCount = 7, CardCountPerPlayer = 7;
	
private:
	bool nextPlayer() override final
	{
		const size_t playerCount = this->players.size();
		if (this->checkPlayerCount(playerCount))
			for (Count count = 1; count < playerCount; ++count)
			{
				if (++this->currentPlayer >= playerCount)
					this->currentPlayer = 0;
				if (!this->players[this->currentPlayer].empty())
					return true;
			}
		return false;
	}
	bool isRealHand(const Hand& hand) const override final
	{
		switch (this->lastHand.type)
		{
		case Type::Single:
		case Type::Pair:
		case Type::PairJokers:
		case Type::Triple:
		case Type::Quadruple:
			return hand.player != INVALID_PLAYER && !hand.cards.empty();
		case Type::Empty:
		case Type::SingleStraight:
		case Type::SingleFlush:
		case Type::SingleFlushStraight:
		case Type::PairStraight:
		case Type::PairStraightWithSingle:
		case Type::TripleWithSingle:
		case Type::TripleWithPair:
		case Type::TripleWithPairSingle:
		case Type::TripleStraight:
		case Type::TripleStraightWithSingle:
		case Type::TripleStraightWithSingles:
		case Type::TripleStraightWithPairs:
		case Type::QuadrupleWithSingle:
		case Type::QuadrupleWithSingleSingle:
		case Type::QuadrupleWithPairPair:
		case Type::QuadrupleStraight:
		case Type::QuadrupleStraightWithSingle:
		case Type::QuadrupleJokers:
		case Type::Quintuple:
		case Type::Sextuple:
		case Type::Septuple:
		case Type::Octuple:
		case Type::Invalid:
		default:
			return false;
		}
	}
	bool coverLastHand(const Hand& currentHand) const override final
	{
		if (this->isRealHand(this->lastHand) && this->isRealHand(currentHand))
			switch (this->lastHand.type)
			{
			case Type::Single:
			case Type::Triple:
			case Type::Quadruple:
				return currentHand.type == this->lastHand.type && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit));
			case Type::Pair:
			case Type::PairJokers:
				return (Type::Pair == currentHand.type || Type::PairJokers == currentHand.type) && (this->values[currentHand.cards[0].point] > this->values[this->lastHand.cards[0].point] || (currentHand.cards[0].point == this->lastHand.cards[0].point && currentHand.cards[0].suit > this->lastHand.cards[0].suit));
			case Type::Empty:
			case Type::SingleStraight:
			case Type::SingleFlush:
			case Type::SingleFlushStraight:
			case Type::PairStraight:
			case Type::PairStraightWithSingle:
			case Type::TripleWithSingle:
			case Type::TripleWithPair:
			case Type::TripleWithPairSingle:
			case Type::TripleStraight:
			case Type::TripleStraightWithSingle:
			case Type::TripleStraightWithSingles:
			case Type::TripleStraightWithPairs:
			case Type::QuadrupleWithSingle:
			case Type::QuadrupleWithSingleSingle:
			case Type::QuadrupleWithPairPair:
			case Type::QuadrupleStraight:
			case Type::QuadrupleStraightWithSingle:
			case Type::QuadrupleJokers:
			case Type::Quintuple:
			case Type::Sextuple:
			case Type::Septuple:
			case Type::Octuple:
			case Type::Invalid:
			default:
				return false;
			}
		else
			return false;
	}
	bool processHand(Hand& hand, std::vector<Candidate>& candidates) const override final
	{
		hand.type = Type::Invalid;
		candidates.clear();
		bool blackJoker = false, redJoker = false;
		std::vector<Count> counts(14);
		for (const Card& card : hand.cards)
			if (JOKER_POINT == card.point)
				switch (card.suit)
				{
				case Suit::Black:
					if (blackJoker)
						return false;
					else
					{
						blackJoker = true;
						++counts[JOKER_POINT];
						break;
					}
				case Suit::Red:
					if (redJoker)
						return false;
					else
					{
						redJoker = true;
						++counts[JOKER_POINT];
						break;
					}
				case Suit::Diamond:
				case Suit::Club:
				case Suit::Heart:
				case Suit::Spade:
				case Suit::Cover:
				default:
					return false;
				}
			else if (this->values[card.point])
				++counts[card.point];
			else
				return false;
		sort(hand.cards.begin(), hand.cards.end(), [&counts, this](const Card a, const Card b) { const Count countA = counts[a.point], countB = counts[b.point]; const Value valueA = this->values[a.point], valueB = this->values[b.point]; return countA > countB || (countA == countB && (valueA > valueB || (valueA == valueB && a.suit > b.suit))); });
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
	bool processBasis(const Hand& hand) override final
	{
		if (Status::Started == this->status && !this->records.empty() && !this->records.back().empty())
		{
			const size_t playerCount = this->players.size();
			if (this->amounts.size() == playerCount && this->checkPlayerCount(playerCount))
				if (this->isRealHand(hand))
				{
					/* Compute the winner bonus */
					if (this->deck.empty() && this->players[hand.player].empty())
					{
						Count winnerCount = 0;
						for (const Amount& amount : this->amounts)
							if (amount >> 8)
								++winnerCount;
						if (winnerCount >= playerCount)
							return false;
						this->amounts[hand.player] += static_cast<Amount>(winnerCount) << 8;
					}
					
					/* Compute the winner integrals */
					if (this->isAbsolutelyLargest(hand))
					{
						/* Scan the integrals */
						Integral integral = 0;
						Count emptyCount = 1;
						for (std::vector<Hand>::const_reverse_iterator it = this->records.back().rbegin() + 1; it != this->records.back().rend(); ++it)
							if (this->isRealHand(*it))
								if (this->isAbsolutelyLargest(*it))
									break;
								else
								{
									for (const Card& card : it->cards)
										switch (card.point)
										{
										case 5:
											integral += 5;
											break;
										case 10:
										case 13:
											integral += 10;
											break;
										default:
											break;
										}
									emptyCount = 1;
								}
							else if (Type::Empty == it->type && it->cards.empty())
							{
								if (++emptyCount >= playerCount)
								{
									if (this->records.back().rend() == ++it || !this->isRealHand(*it))
										return false;
									else
										break;
								}
							}
							else
								return false;
						
						/* Apply the amount */
						Player bonusPlayer = hand.player;
						this->amounts[bonusPlayer] += integral;
						
						/* Draw */
						for (Count count = 0; count < playerCount; ++count)
						{
							while (this->players[bonusPlayer].size() < Qiguiwuersan::CardCountPerPlayer)
								if (this->deck.empty())
									return true;
								else
								{
									this->players[bonusPlayer].emplace_back(std::move(this->deck.back()));
									this->deck.pop_back();
								}
							this->sortCards(this->players[bonusPlayer]);
							if (++bonusPlayer >= playerCount)
								bonusPlayer = 0;
						}
					}
					return true;
				}
				else if (Type::Empty == hand.type && hand.cards.empty()) // Compute the winner integrals
				{
					/* Judge an end of a circle */
					std::vector<Hand>::const_reverse_iterator it = this->records.back().rbegin();
					for (Count emptyCount = 2; emptyCount < playerCount;)
						if (this->records.back().rend() == ++it || this->isRealHand(*it))
							return true;
						else if (Type::Empty == it->type && it->cards.empty())
							++emptyCount;
						else
							return false;
					if (this->records.back().rend() == ++it || !this->isRealHand(*it))
						return false;
					
					/* Scan the integrals */
					Integral integral = 0;
					Player bonusPlayer = it->player;
					for (const Card& card : it->cards)
						switch (card.point)
						{
						case 5:
							integral += 5;
							break;
						case 10:
						case 13:
							integral += 10;
							break;
						default:
							break;
						}
					for (Count emptyCount = 1; ++it != this->records.back().rend();)
						if (this->isRealHand(*it))
							if (this->isAbsolutelyLargest(*it))
								break;
							else
							{
								for (const Card& card : it->cards)
									switch (card.point)
									{
									case 5:
										integral += 5;
										break;
									case 10:
									case 13:
										integral += 10;
										break;
									default:
										break;
									}
								emptyCount = 1;
							}
						else if (Type::Empty == it->type && it->cards.empty())
						{
							if (++emptyCount >= playerCount)
							{
								if (this->records.back().rend() == ++it || !this->isRealHand(*it))
									return false;
								else
									break;
							}
						}
						else
							return false;
					
					/* Apply the amount */
					this->amounts[bonusPlayer] += integral;
					
					/* Draw */
					for (Count count = 0; count < playerCount; ++count)
					{
						while (this->players[bonusPlayer].size() < Qiguiwuersan::CardCountPerPlayer)
							if (this->deck.empty())
								return true;
							else
							{
								this->players[bonusPlayer].emplace_back(std::move(this->deck.back()));
								this->deck.pop_back();
							}
						this->sortCards(this->players[bonusPlayer]);
						if (++bonusPlayer >= playerCount)
							bonusPlayer = 0;
					}
					return true;
				}
				else
					return false;
			else
				return false;
		}
		else
			return false;
	}
	bool isOver() const override final
	{
		if (this->status >= Status::Started && this->deck.empty())
		{
			bool hasCards = false;
			for (const std::vector<Card>& cards : this->players)
				if (!cards.empty())
				{
					if (hasCards)
						return false;
					else
						hasCards = true;
				}
			return true;
		}
		return false;
	}
	bool isAbsolutelyLargest(const Hand& hand) const override final
	{
		return (Type::Single == hand.type || Type::Pair == hand.type || Type::Triple == hand.type || Type::Quadruple == hand.type) && (!hand.cards.empty() && Card { 7, Suit::Spade } == hand.cards[0]);
	}
	std::string getBasisString() const override final
	{
		const size_t playerCount = this->players.size();
		if (playerCount == this->amounts.size() && this->checkPlayerCount(playerCount))
		{
			std::string basisString = "积分信息：";
			bool flag = false;
			for (Player player = 0; player < playerCount; ++player)
			{
				Amount upperAmount = this->amounts[player] >> 8, lowerAmount = this->amounts[player] & 0xFF/* 0b11111111*/;
				if (upperAmount)
				{
					if (flag)
						basisString += "；";
					basisString += "玩家 " + std::to_string(player + 1) + " 是第 " + std::to_string(upperAmount) + " 位出完牌的玩家";
					if (lowerAmount)
						basisString += "，得 " + std::to_string(lowerAmount) + " 积分";
					flag = true;
				}
				else if (lowerAmount)
				{
					if (flag)
						basisString += "；";
					basisString += "玩家 " + std::to_string(player + 1) + " 得 " + std::to_string(lowerAmount) + " 积分";
					flag = true;
				}
			}
			basisString += flag ? "。\n" : "暂无玩家获得积分或出完牌。\n";
			return basisString;
		}
		else
			return "";
	}
	std::string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			std::string preRoundString{};
			for (const Hand& hand : this->records[0])
				if (hand.cards.size() == 1)
					preRoundString += (std::string)hand.cards[0] + "（玩家 " + std::to_string(hand.player + 1) + "） > ";
				else
					return "预备回合信息检验异常。";
			preRoundString.erase(preRoundString.length() - 3, 3);
			return preRoundString;
		}
	}
	std::string getAmountString() const override final
	{
		const size_t playerCount = this->players.size();
		if (Status::Over == this->status && playerCount == this->amounts.size() && this->checkPlayerCount(playerCount))
		{
			/* Compress rankings */
			std::vector<Player> sortedPlayers(playerCount);
			std::vector<Amount> upperAmounts(playerCount), lowerAmounts(playerCount);
			for (Player player = 0; player < playerCount; ++player)
			{
				sortedPlayers[player] = player;
				upperAmounts[player] = this->amounts[player] >> 8;
				lowerAmounts[player] = this->amounts[player] & 0xFF/* 0b11111111*/;
			}
			sort(sortedPlayers.begin(), sortedPlayers.end(), [&lowerAmounts](const Player playerA, const Player playerB) {return lowerAmounts[playerA] > lowerAmounts[playerB]; });
			std::vector<Ranking> rankings(playerCount);
			rankings[sortedPlayers[0]] = 1;
			Ranking zippedRanking = 1, ranking = 1;
			for (size_t idx = 1; idx < playerCount; ++idx)
			{
				if (lowerAmounts[sortedPlayers[idx - 1]] == lowerAmounts[sortedPlayers[idx]])
					++zippedRanking;
				else
				{
					ranking += zippedRanking;
					zippedRanking = 1;
				}
				rankings[sortedPlayers[idx]] = ranking;
			}
			
			/* Output rankings */
			std::string amountString = "/* 结算信息 */\n";
			bool flag = false;
			for (Player player = 0; player < playerCount; ++player)
				if (upperAmounts[player])
				{
					if (flag)
						amountString += "；";
					amountString += "玩家 " + std::to_string(player + 1) + " 是第 " + std::to_string(upperAmounts[player]) + " 位出完牌的玩家";
					if (lowerAmounts[player])
						amountString += "，得 " + std::to_string(lowerAmounts[player]) + " 积分，积分排名为 " + std::to_string(rankings[player]);
					flag = true;
				}
				else if (lowerAmounts[player])
				{
					if (flag)
						amountString += "；";
					amountString += "玩家 " + std::to_string(player + 1) + " 得 " + std::to_string(lowerAmounts[player]) + " 积分，积分排名为 " + std::to_string(rankings[player]);
					flag = true;
				}
			amountString += flag ? "。\n" : "结算信息异常，请各位玩家自行计算结算信息。\n";
			return amountString;
		}
		else
			return "结算信息异常，请各位玩家自行计算结算信息。\n";
	}
	
protected:
	bool checkPlayerCount(const size_t playerCount) const override final
	{
		return Qiguiwuersan::MinimumPlayerCount <= playerCount && playerCount <= Qiguiwuersan::MaximumPlayerCount;
	}
	
public:
	Qiguiwuersan() : Poker()
	{
		this->name = "七鬼五二三";
	}
	bool initialize() override final { return this->initialize(Qiguiwuersan::MinimumPlayerCount); }
	bool initialize(const size_t playerCount) override
	{
		if (this->status >= Status::Ready && this->checkPlayerCount(playerCount))
		{
			Value value = 1;
			this->values.set(4, value++);
			this->values.set(6, value++);
			for (Point point = 8; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(3, value++);
			this->values.set(2, value++);
			this->values.set(5, value++);
			this->values.set(JOKER_POINT, value++);
			this->values.set(7, value++);
			this->players = std::vector<std::vector<Card>>(playerCount);
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
	bool deal() override final
	{
		const size_t playerCount = this->players.size();
		if (this->status >= Status::Initialized && this->checkPlayerCount(playerCount))
		{
			this->deck.clear();
			this->add54CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < playerCount; ++player)
			{
				this->players[player] = std::vector<Card>(Qiguiwuersan::CardCountPerPlayer);
				for (size_t idx = 0; idx < Qiguiwuersan::CardCountPerPlayer; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records = std::vector<std::vector<Hand>>{ std::vector<Hand>{} };
			for (Player player = 0; player < playerCount; ++player)
				this->records[0].push_back(Hand{ player, std::vector<Card>{ this->players[player].back() } });
			sort(this->records[0].begin(), this->records[0].end(), [this](Hand a, Hand b) { const Value valueA = this->values[a.cards.back().point], valueB = this->values[b.cards.back().point]; return valueA > valueB || (valueA == valueB && a.cards.back().suit > b.cards.back().suit); });
			this->currentPlayer = this->records[0].back().player;
			this->dealer = this->records[0].back().player;
			this->lastHand = Hand{};
			this->amounts = std::vector<Amount>(playerCount);
			this->status = Status::Assigned;
			return true;
		}
		else
			return false;
	}
	bool display(const std::vector<Player>& selectedPlayers) const override final
	{
		return Poker::display(selectedPlayers, "最小先出", "牌堆（自下往上）：" + this->cards2string(this->deck, "", " | ", "", "（空）") + "\n\n");
	}
};

class Qiguiwueryi : public Qiguiwuersan /* Previous: Qiguiwuersan */
{
public:
	Qiguiwueryi() : Qiguiwuersan()
	{
		this->name = "七鬼五二一";
	}
	bool initialize(const size_t playerCount) override final
	{
		if (this->status >= Status::Ready && this->checkPlayerCount(playerCount))
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
			this->players = std::vector<std::vector<Card>>(playerCount);
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
	const std::vector<std::string> helpOptions = { "?", "/?", "-?", "h", "/h", "-h", "help", "/help", "--help" };
	HelpKey helpKey = 0;
	const std::vector<std::string> nameOptions = { "n", "/n", "-n", "name", "/name", "--name" };
	const std::vector<std::string> namesC = { "斗地主", "斗地主拓展版", "四人斗地主", "锄大地", "三两一", "五瓜皮", "七鬼五二一", "七鬼五二三" };
	const std::vector<std::string> namesE = { "Landlords", "LandlordsX", "Landlords4P", "BigTwo", "ThreeTwoOne", "Wuguapi", "Qiguiwueryi", "Qiguiwuersan" };
	std::string name = "扑克牌";
	const std::vector<std::string> playerCountOptions = { "p", "/p", "-p", "playerCount", "/playerCount", "--playerCount" };
	size_t playerCount = 0;
	const std::vector<std::string> displayOptions = { "d", "/d", "-d", "display", "/display", "--display" };
	std::vector<Order> orders{};
	Poker* poker = nullptr;
	const std::vector<std::string> landlordStatements = { "Y", "yes", "1", "T", "true", "是", "叫", "叫地主", "叫牌", "抢", "抢地主", "抢牌" };
	const std::vector<std::string> againStatements = { "Again", "再来", "再来一局", "新开", "新开一局" };
	const std::vector<std::string> returnStatements = { "Return", "返回", "返回主面板", "返回主界面" };
	const std::vector<std::string> exitStatements = { "Exit", "Ctrl+C", "Ctrl + C", "退出", "退出程序" };
	
	/* Command line handling */
	bool isEqual(const std::string& s1, const std::string& s2) const // Please use == directly if cases cannot be ignored
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
	bool isIn(const std::string& s, const std::vector<std::string>& strings) const
	{
		for (const std::string& str : strings)
			if (this->isEqual(s, str))
				return true;
		return false;
	}
	void replaceAll(std::string& str, const std::string& oldSubString, const std::string& newSubString) const
	{
		size_t pos = 0;
		while ((pos = str.find(oldSubString, pos)) != std::string::npos)
		{
			str.replace(pos, oldSubString.length(), newSubString);
			pos += newSubString.length();
		}
		return;
	}
	void optimizePokerType(std::string& _name) const
	{
		if (!this->isIn(_name, this->namesE))
		{
			_name.erase(remove_if(_name.begin(), _name.end(), [](char ch) { return ' ' == ch || '\t' == ch || '\r' == ch || '\n' == ch; }), _name.end());
			this->replaceAll(_name, "扑克游戏", "");
			this->replaceAll(_name, "扑克", "");
			this->replaceAll(_name, "X", "拓展版");
			this->replaceAll(_name, "D", "地");
			this->replaceAll(_name, "7", "七");
			this->replaceAll(_name, "王", "鬼");
			this->replaceAll(_name, "5", "五");
			this->replaceAll(_name, "3", "三");
			this->replaceAll(_name, "2", "二");
			this->replaceAll(_name, "1", "一");
			this->replaceAll(_name, "三人斗地主", "斗地主");
			this->replaceAll(_name, "欢乐斗地主", "斗地主");
			this->replaceAll(_name, "竞技二打一", "斗地主");
			this->replaceAll(_name, "二打一", "斗地主");
			this->replaceAll(_name, "扩展版", "拓展版");
			this->replaceAll(_name, "大老二", "锄大地");
			this->replaceAll(_name, "锄大弟", "锄大地");
		}
		return;
	}
	std::string vector2string(const std::vector<std::string>& strings, const std::string& prefix, const std::string& separator, const std::string& suffix) const
	{
		std::string stringBuffer = prefix;
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
	bool printHelp()
	{
		std::cout << "通用扑克游戏实现程序。" << std::endl << std::endl;
		switch (this->helpKey)
		{
		case 'T':
		case 't':
		{
			std::cout << "目前支持以下扑克游戏：" << std::endl;
			const size_t length = this->namesC.size();
			for (size_t idx = 0; idx < length; ++idx)
				std::cout << "\t（" << (idx + 1) << "）" << this->namesC[idx] << "；" << std::endl;
			std::cout << std::endl;
			return true;
		}
		case 'P':
		case 'p':
			std::cout << "用于指定玩家人数的参数目前仅对以下扑克游戏生效：" << std::endl;
			std::cout << "\t（1）五瓜皮：最少 2 人，最多 10 人，默认 2 人；" << std::endl;
			std::cout << "\t（2）七鬼五二一：最少 2 人，最多 7 人，默认 2 人；" << std::endl;
			std::cout << "\t（3）七鬼五二三：最少 2 人，最多 7 人，默认 2 人。" << std::endl << std::endl;
			std::cout << "否则，该参数将会被自动忽略。" << std::endl << std::endl;
			return true;
		case 'S':
		case 's':
			std::cout << "目前支持以下十四项单级排序：" << std::endl;
			std::cout << "\t（01）P：依照点数升序（0b01010000）；" << std::endl;
			std::cout << "\t（02）V：依照价值升序（0b01010110）；" << std::endl;
			std::cout << "\t（03）S：依照花色升序（0b01010011）；" << std::endl;
			std::cout << "\t（04）H：依照点数的计数升序（0b01001000 = 'P' - 0b1000）；" << std::endl;
			std::cout << "\t（05）M：依照扑克牌的计数升序（0b01001101 = 'U' - 0b1000）；" << std::endl;
			std::cout << "\t（06）N：依照价值的计数升序（0b01001110 = 'V' - 0b1000）；" << std::endl;
			std::cout << "\t（07）K：依照花色的计数升序（0b01001011 = 'S' - 0b1000）；" << std::endl;
			std::cout << "\t（08）p：依照点数降序（0b01110000）；" << std::endl;
			std::cout << "\t（09）v：依照价值降序（0b01110110）；" << std::endl;
			std::cout << "\t（10）s：依照花色降序（0b01110011）；" << std::endl;
			std::cout << "\t（11）h：依照点数的计数降序（0b01101000 = 'p' - 0b1000）；" << std::endl;
			std::cout << "\t（12）m：依照扑克牌的计数降序（0b01101101 = 'u' - 0b1000）；" << std::endl;
			std::cout << "\t（13）n：依照价值的计数降序（0b01101110 = 'v' - 0b1000）；" << std::endl;
			std::cout << "\t（14）k：依照花色的计数降序（0b01101011 = 's' - 0b1000）。" << std::endl << std::endl;
			std::cout << "请注意：" << std::endl;
			std::cout << "\t（1）该排序仅对玩家手上的扑克牌生效，对于每级排序，所调用的排序函数均使用不稳定排序算法；" << std::endl;
			std::cout << "\t（2）您可以使用至少一项、至多七项单级排序依照您所期望的顺序来实现单级或多级排序，例如，使用默认值“vs”将先按价值降序，再按花色降序；" << std::endl;
			std::cout << "\t（3）您可以使用二进制（以“0b”作为前缀）、八进制（以“0o”或“0”作为前缀）、十进制或十六进制（以“0x”作为前缀）来替代对应字母，若为多级排序，请仅保留值最前方的前缀，例如使用 0b0111011001110011 替代“vs”；" << std::endl;
			std::cout << "\t（4）使用字母指示一级排序时，该字母须严格区分大小写，使用带有前缀的数值时，前缀可不区分大小写；" << std::endl;
			std::cout << "\t（5）同一字母的大小写不能同时出现在多级排序中，因为它们通常没有意义，例如，您不能使用“vsV”，因为在按价值降序后传递给后续每级排序的两个 Card 类型变量均满足价值相等，这会导致后续的按价值升序对比两个 Card 类型变量的结果恒定为相等。" << std::endl << std::endl;
			return true;
		default:
			std::cout << "参数：" << std::endl;
			std::cout << "\t" << this->vector2string(this->nameOptions, "[", "|", "]") << " [扑克游戏]\t\t\t\t\t\t\t设置扑克游戏" << std::endl;
			std::cout << "\t" << this->vector2string(this->playerCountOptions, "[", "|", "]") << " [玩家人数]\t\t\t\t设置玩家人数" << std::endl;
			std::cout << "\t" << this->vector2string(this->displayOptions, "[", "|", "]") << " [排序显示方式]\t\t\t\t\t设置排序显示方式" << std::endl;
			std::cout << "\t" << this->vector2string(this->helpOptions, "[", "|", "]") << " 或 [其它参数] " << this->vector2string(this->helpOptions, "[", "|", "]") << "\t显示帮助" << std::endl << std::endl;
			std::cout << "注意：" << std::endl;
			std::cout << "\t（1）键和值应当成对出现，即每一个表示键的参数后均应紧接着其对应的值（含帮助参数）；" << std::endl;
			std::cout << "\t（2）不存在重复的键时，键值对的顺序不影响程序对命令行的解析，除以字母指示的排序外，所有键和值均不区分大小写；" << std::endl;
			std::cout << "\t（3）当同一键出现多次时，其值以该键最后一次以合法键值对的形式出现时的值为准；" << std::endl;
			std::cout << "\t（4）出现多个帮助参数时，以最后一次出现帮助参数时的上下文进行显示帮助。" << std::endl << std::endl;
			return 1 == this->helpKey;
		}
	}
	
	/* Interactive handling */
	int clearScreen() const
	{
#if defined _WIN32 || defined _WIN64
		return system("cls");
#else
		return system("clear");
#endif
	}
	void rfstdin() const
	{
		rewind(stdin);
		fflush(stdin);
		return;
	}
	void getDescription(std::string& description) const
	{
		this->rfstdin();
		description.clear();
		char c = 0;
		while (std::cin.get(c) && c != '\n')
			description += c;
		return;
	}
	size_t fetchPlayerCount(const size_t _lowerBound, const size_t _upperBound) const // The two boundaries must be within the boundaries of ``Poker``. 
	{
		const size_t lowerBound = Poker::MinimumPlayerCount <= _lowerBound && _lowerBound <= Poker::MaximumPlayerCount ? _lowerBound : Poker::MinimumPlayerCount, upperBound = Poker::MinimumPlayerCount <= _upperBound && _upperBound <= Poker::MaximumPlayerCount ? _upperBound : Poker::MaximumPlayerCount;
		if (lowerBound <= this->playerCount && this->playerCount <= upperBound)
			return this->playerCount;
		else
		{
			this->clearScreen();
			std::cout << "已选定扑克游戏为" << this->name << "。" << std::endl;
			std::cout << "该扑克游戏支持最少 " << lowerBound << " 人，最多 " << upperBound << " 人。" << std::endl;
			for (;;)
			{
				std::string playerCountString{};
				std::cout << "请输入玩家人数（输入“" << DEFAULT_STRING << "”并按下回车键将使用默认值）：";
				this->getDescription(playerCountString);
				if (DEFAULT_STRING == playerCountString)
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
	bool fetchPokerType()
	{
		this->optimizePokerType(this->name);
		if (!this->isIn(this->name, this->namesC) && !this->isIn(this->name, this->namesE))
			for (;;)
			{
				this->clearScreen();
				std::cout << "可选的扑克游戏如下：" << std::endl;
				const size_t length = this->namesC.size();
				for (size_t idx = 0; idx < length; ++idx)
					std::cout << "\t" << (idx + 1) << " = " << this->namesC[idx] << std::endl;
				std::cout << "\t0 = 退出程序" << std::endl << std::endl << "请选择或输入一种扑克牌以继续：";
				std::string buffer{};
				this->getDescription(buffer);
				if (buffer.size() == 1)
					if ('0' == buffer.at(0))
						return false;
					else
					{
						const size_t choice = static_cast<size_t>(buffer.at(0) - '1');
						if (choice < length)
						{
							this->name = this->namesC[choice];
							break;
						}
					}
				else
				{
					this->optimizePokerType(buffer);
					if (this->isIn(buffer, this->namesC) || this->isIn(buffer, this->namesE))
					{
						this->name = buffer;
						break;
					}
					else if ("退出程序" == buffer || this->isEqual("Exit", buffer))
						return false;
				}
			}
		if ("五瓜皮" == this->name || this->isEqual("Wuguapi", this->name))
			this->playerCount = fetchPlayerCount(2, 10);
		else if ("七鬼五二一" == this->name || this->isEqual("Qiguiwueryi", this->name) || "七鬼五二三" == this->name || this->isEqual("Qiguiwuersan", this->name))
			this->playerCount = fetchPlayerCount(2, 7);
		else
			this->playerCount = 0;
		return true;
	}
	bool fetchBinaryChars(const std::string& filePath, std::vector<char>& binaryChars) const // return true if the parameter is a good file to be read
	{
		std::ifstream ifs(filePath, std::ios::binary | std::ios::ate);
		if (ifs.is_open())
		{
			std::streampos fileSize = ifs.tellg();
			ifs.seekg(0, std::ios::beg);
			binaryChars = std::vector<char>((size_t)fileSize);
			const bool flag = static_cast<bool>(ifs.read(binaryChars.data(), fileSize));
			ifs.close();
			return flag;
		}
		else
			return false;
	}

	/* Action confirmation */
	bool ensureAction(const std::string& buffer, const std::string& actionDescriptioin) const
	{
		std::string repeatedBuffer{};
		std::cout << "您确定要" << actionDescriptioin << "吗？请再次输入以上指令以确认：";
		this->getDescription(repeatedBuffer);
		return buffer == repeatedBuffer;
	}
	bool controlAction(const std::string& buffer, Action& action) const
	{
		if (this->isIn(buffer, this->againStatements))
			if (this->ensureAction(buffer, "新开一局"))
			{
				action = Action::AgainConfirmed;
				return true;
			}
			else
			{
				action = Action::AgainCancelled;
				return false;
			}
		else if (this->isIn(buffer, this->returnStatements))
			if (this->ensureAction(buffer, "返回主界面"))
			{
				action = Action::ReturnConfirmed;
				return true;
			}
			else
			{
				action = Action::ReturnCancelled;
				return false;
			}
		else if (this->isIn(buffer, this->exitStatements))
			if (this->ensureAction(buffer, "退出程序"))
			{
				action = Action::ExitConfirmed;
				return true;
			}
			else
			{
				action = Action::ExitCancelled;
				return false;
			}
		else
			return false;
	}

	/* Procedures */
	bool setLandlord(Action& action) const
	{
		Player player = INVALID_PLAYER;
		std::string buffer{};
		Count retryCount = 0;
		for (;;)
		{
			bool isRobbing = false;
			Count callerAndRobberCount = 0;
			for (Count count = 0; count < 3;)
			{
				this->poker->getCurrentPlayer(player);
				this->clearScreen();
				this->poker->display(this->orders.empty() ? INVALID_PLAYER : player);
				std::cout << "请玩家 " << (player + 1) << " 选择是否" << (isRobbing ? "抢" : "叫") << "地主：";
				this->getDescription(buffer);
				if (this->controlAction(buffer, action))
					return false;
				else if (action > Action::None)
					action = Action::None;
				else if (!buffer.empty())
				{
					if (this->isIn(buffer, this->landlordStatements))
					{
						if (this->poker->setLandlord(true))
						{
							isRobbing = true;
							++callerAndRobberCount;
							++count;
						}
					}
					else if (this->poker->setLandlord(false))
						++count;
				}
			}
			if (callerAndRobberCount >= 2)
				for (;;)
				{
					this->poker->getCurrentPlayer(player);
					this->clearScreen();
					this->poker->display(this->orders.empty() ? INVALID_PLAYER : player);
					std::cout << "请玩家 " << (player + 1) << " 选择是否抢地主：";
					this->getDescription(buffer);
					if (this->controlAction(buffer, action))
						return false;
					else if (action > Action::None)
						action = Action::None;
					else if (!buffer.empty() && this->poker->setLandlord(this->isIn(buffer, this->landlordStatements)))
						break;
				}
			else if (0 == callerAndRobberCount && ++retryCount < 3)
			{
				std::cout << "无人叫地主，即将重新发牌。" << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(TIME_FOR_SLEEP));
				this->poker->deal();
				continue;
			}
			break;
		}
		return true;
	}
	bool setLandlord4P(Action& action) const
	{
		Player player = INVALID_PLAYER;
		std::string buffer{};
		Count retryCount = 0;
		for (;;)
		{
			Score currentHighestScore = Score::None;
			std::vector<std::string> scoreDescriptions{ "不叫", "3分", "2分", "1分" };
			for (Count count = 1; count <= 4 && currentHighestScore < Score::Three;)
			{
				this->poker->getCurrentPlayer(player);
				this->clearScreen();
				this->poker->display(this->orders.empty() ? INVALID_PLAYER : player);
				std::cout << "请玩家 " << (player + 1) << " 选择（" << this->vector2string(scoreDescriptions, "", " | ", "") << "）：";
				this->getDescription(buffer);
				if (this->controlAction(buffer, action))
					return false;
				else if (action > Action::None)
					action = Action::None;
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
					if (this->poker->setLandlord(score))
					{
						if (score > currentHighestScore)
						{
							for (Count removalCount = static_cast<Count>(static_cast<Count>(score) - static_cast<Count>(currentHighestScore)); removalCount > 0; --removalCount)
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
				std::cout << "无人叫地主，即将重新发牌。" << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(TIME_FOR_SLEEP));
				this->poker->deal();
			}
		}
		return true;
	}
	void resetAll()
	{
		this->helpKey = 0;
		this->name = "扑克牌";
		this->playerCount = 0;
		this->orders = std::vector<Order>{};
		if (this->poker != nullptr)
		{
			delete this->poker;
			this->poker = nullptr;
		}
		return;
	}
	bool selectType(std::vector<Candidate>& candidates, Action& action) const
	{
		std::cout << "当前牌组存在二义性，在当前环境中，所有可能的牌型列举如下：" << std::endl;
		const size_t length = candidates.size();
		for (size_t idx = 0; idx < length; ++idx)
			std::cout << "\t" << (idx + 1) << " = " << candidates[idx].description << std::endl;
		for (;;)
		{
			std::cout << std::endl << "请选择一种牌型以继续（输入“" << DEFAULT_STRING << "”并按下回车键将重新选择要出的牌）：";
			std::string buffer{};
			this->getDescription(buffer);
			if (this->controlAction(buffer, action) || DEFAULT_STRING == buffer)
				return false;
			else if (action > Action::None)
				action = Action::None;
			else if (!buffer.empty())
			{
				const size_t choice = static_cast<size_t>(strtoul(buffer.c_str(), nullptr, 0) - 1);
				if (choice < length)
				{
					candidates = std::vector<Candidate>{ candidates[choice] };
					return true;
				}
			}
		}
	}
	bool start(Action& action) const
	{
		Player player = INVALID_PLAYER;
		std::string buffer{};
		this->poker->getCurrentPlayer(player);
		for (;;)
		{
			this->clearScreen();
			this->poker->display(this->orders.empty() ? INVALID_PLAYER : player);
			std::cout << "请玩家 " << (player + 1) << " 开牌：";
			this->getDescription(buffer);
			if (this->controlAction(buffer, action))
				return false;
			else if (action > Action::None)
				action = Action::None;
			else if (!buffer.empty())
			{
				std::vector<Candidate> candidates{};
				if (this->poker->start(buffer, candidates))
					return true;
				else if (candidates.empty())
				{
					std::cout << "无法使用所选牌组开牌，请重新选择要出的牌。" << std::endl;
					std::this_thread::sleep_for(std::chrono::seconds(TIME_FOR_SLEEP));
				}
				else if (this->selectType(candidates, action))
				{
					if (this->poker->start(candidates[0].hand.cards, candidates))
						return true;
				}
				else if (action != Action::None)
					return false;
			}
		}
	}
	bool play(Action& action) const
	{
		Player player = INVALID_PLAYER;
		std::string buffer{};
		this->poker->getCurrentPlayer(player);
		while (player != INVALID_PLAYER)
		{
			for (;;)
			{
				this->clearScreen();
				this->poker->display(this->orders.empty() ? INVALID_PLAYER : player);
				std::cout << "请玩家 " << (player + 1) << " 出牌：";
				this->getDescription(buffer);
				if (this->controlAction(buffer, action))
					return false;
				else if (action > Action::None)
					action = Action::None;
				else if (!buffer.empty())
				{
					std::vector<Candidate> candidates{};
					if (this->poker->play(buffer, candidates))
						break;
					else if (candidates.empty())
					{
						std::cout << "无法使用所选牌组出牌，请重新选择要出的牌。" << std::endl;
						std::this_thread::sleep_for(std::chrono::seconds(TIME_FOR_SLEEP));
					}
					else if (this->selectType(candidates, action))
					{
						if (this->poker->play(candidates[0].hand.cards, candidates))
							break;
					}
					else if (action != Action::None)
						return false;
				}
			}
			this->poker->getCurrentPlayer(player);
		}
		return true;
	}
	
public:
	Interaction()
	{

	}
	Interaction(const std::vector<std::string>& arguments)
	{
		if (!arguments.empty())
		{
			std::vector<size_t> invalidArgumentIndexes{};
			const size_t argumentCount = arguments.size() - 1;
			size_t argumentID = 0;
			for (; argumentID < argumentCount; ++argumentID)
				if (this->isIn(arguments[argumentID], this->helpOptions))
					this->helpKey = 1;
				else if (this->isIn(arguments[argumentID], this->nameOptions))
					if (this->isIn(arguments[++argumentID], this->helpOptions))
						this->helpKey = 't';
					else
					{
						std::string _name = arguments[argumentID];
						this->optimizePokerType(_name);
						if (this->isIn(_name, this->namesC) || this->isIn(_name, this->namesE))
							this->name = _name;
						else
							invalidArgumentIndexes.push_back(argumentID);
					}
				else if (this->isIn(arguments[argumentID], this->playerCountOptions))
					if (this->isIn(arguments[++argumentID], this->helpOptions))
						this->helpKey = 'p';
					else
					{
						const size_t expectedPlayerCount = (size_t)strtoul(arguments[argumentID].c_str(), NULL, 0);
						if (Poker::MinimumPlayerCount <= expectedPlayerCount && expectedPlayerCount <= Poker::MaximumPlayerCount)
							this->playerCount = expectedPlayerCount;
						else
							invalidArgumentIndexes.push_back(argumentID);
					}
				else if (this->isIn(arguments[argumentID], this->displayOptions))
					if (this->isIn(arguments[++argumentID], this->helpOptions))
						this->helpKey = 's';
					else
						invalidArgumentIndexes.push_back(argumentID);
				else
					invalidArgumentIndexes.push_back(argumentID);
			if (argumentID == argumentCount)
			{
				if (this->isIn(arguments[argumentID], this->helpOptions))
					this->helpKey = 1;
				else
					invalidArgumentIndexes.push_back(argumentID);
			}
			if (!invalidArgumentIndexes.empty())
			{
				const size_t length = invalidArgumentIndexes.size();
				std::cout << "警告：以下 " << length << " 个参数无效。" << std::endl;
				for (size_t idx = 0; idx < length; ++idx)
					std::cout << "（" << (idx + 1) << "）参数 " << (invalidArgumentIndexes[idx] + 1) << " 无效——“" << arguments[invalidArgumentIndexes[idx]] << "”，其键值对（如有）已被自动跳过。" << std::endl;
				std::cout << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(TIME_FOR_SLEEP * length));
			}
		}
	}
	bool interact()
	{
		if (this->helpKey)
			return this->printHelp();
		else
			for (;;)
			{
				this->poker = nullptr;
				while (nullptr == this->poker)
				{
					if ("斗地主" == this->name || this->isEqual("Landlords", this->name)) // "斗地主"
						this->poker = new Landlords;
					else if ("斗地主拓展版" == this->name || this->isEqual("LandlordsX", this->name)) // "斗地主拓展版"
						this->poker = new LandlordsX;
					else if ("四人斗地主" == this->name || this->isEqual("Landlords4P", this->name)) // "四人斗地主"
						this->poker = new Landlords4P;
					else if ("锄大地" == this->name || this->isEqual("BigTwo", this->name)) // "锄大地"
						this->poker = new BigTwo;
					else if ("三两一" == this->name || this->isEqual("ThreeTwoOne", this->name)) // "三两一"
						this->poker = new ThreeTwoOne;
					else if ("五瓜皮" == this->name || this->isEqual("Wuguapi", this->name)) // "五瓜皮"
						this->poker = new Wuguapi;
					else if ("七鬼五二三" == this->name || this->isEqual("Qiguiwuersan", this->name)) // "七鬼五二三"
						this->poker = new Qiguiwuersan;
					else if ("七鬼五二一" == this->name || this->isEqual("Qiguiwueryi", this->name)) // "七鬼五二一"
						this->poker = new Qiguiwueryi;
					else if (!this->fetchPokerType())
						return true;
				}
				this->fetchPokerType();
				if (this->playerCount ? this->poker->initialize(this->playerCount) : this->poker->initialize())
				{
					/* Beginning */
					this->clearScreen();
					std::cout << "当前牌局（" << this->name << "）已初始化，但暂未开局，请发牌或录入残局数据。" << std::endl << "请输入“" << DEFAULT_STRING << "”并按下回车键开局，或录入残局库数据：";
					for (;;)
					{
						std::string buffer{};
						this->getDescription(buffer);
						if (DEFAULT_STRING == buffer)
							if (this->poker->deal())
								break;
							else
								std::cout << "开局失败！请再次尝试输入“" << DEFAULT_STRING << "”并按下回车键开局，或录入残局库数据：";
						else
						{
							std::vector<char> binaryChars{};
							this->fetchBinaryChars(buffer, binaryChars);
							if (this->poker->set(binaryChars))
								break;
							else
								std::cout << "录入失败！请输入“" << DEFAULT_STRING << "”并按下回车键开局，或再次尝试录入残局库数据：";
						}
					}
					
					/* Poker::setLandlord(4P) */
					Action action = Action::None;
					if ("斗地主" == this->name)
						this->setLandlord(action);
					else if ("四人斗地主" == this->name)
						this->setLandlord4P(action);
					switch (action)
					{
					case Action::ReturnConfirmed:
						this->resetAll();
#if ((defined _MSVC_LANG && _MSVC_LANG >= 201703L) || (!defined _MSVC_LANG && defined __cplusplus && __cplusplus >= 201103L))
						[[fallthrough]];
#endif
					case Action::AgainConfirmed:
						continue;
					case Action::ExitConfirmed:
						return true;
					case Action::None:
					case Action::AgainCancelled:
					case Action::ReturnCancelled:
					case Action::ExitCancelled:
					default:
						break;
					}
					
					/* Poker::start */
					this->start(action);
					switch (action)
					{
					case Action::ReturnConfirmed:
						this->resetAll();
#if ((defined _MSVC_LANG && _MSVC_LANG >= 201703L) || (!defined _MSVC_LANG && defined __cplusplus && __cplusplus >= 201103L))
						[[fallthrough]];
#endif
					case Action::AgainConfirmed:
						continue;
					case Action::ExitConfirmed:
						return true;
					case Action::None:
					case Action::AgainCancelled:
					case Action::ReturnCancelled:
					case Action::ExitCancelled:
					default:
						break;
					}
					
					/* Poker::play */
					this->play(action);
					switch (action)
					{
					case Action::ReturnConfirmed:
						this->resetAll();
#if ((defined _MSVC_LANG && _MSVC_LANG >= 201703L) || (!defined _MSVC_LANG && defined __cplusplus && __cplusplus >= 201103L))
						[[fallthrough]];
#endif
					case Action::AgainConfirmed:
						continue;
					case Action::ExitConfirmed:
						return true;
					case Action::None:
					case Action::AgainCancelled:
					case Action::ReturnCancelled:
					case Action::ExitCancelled:
					default:
						break;
					}

					/* Ending */
					this->clearScreen();
					this->poker->display();
					std::cout << "此局已终，请按 A 键回车再来一局，按 E 键回车退出程序，按其它键回车更改扑克游戏：";
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
					std::cout << "错误：初始化实例失败。" << std::endl << std::endl << std::endl << std::endl;
					this->resetAll();
					return false;
				}
			}
	}
	Interaction(const Interaction&) = delete;
	Interaction(Interaction&&) = delete;
	Interaction& operator=(const Interaction&) = delete;
	Interaction& operator=(Interaction&&) = delete;
};



int main(int argc, char* argv[])
{
	if (argc >= 2)
	{
		const size_t argumentCount = static_cast<size_t>(argc) - 1;
		std::vector<std::string> arguments(argumentCount);
		for (size_t i = 0; i < argumentCount; ++i)
			arguments[i] = argv[i + 1];
		Interaction interaction(arguments);
		return interaction.interact() ? EXIT_SUCCESS : EXIT_FAILURE;
	}
	else
	{
		Interaction interaction{};
		return interaction.interact() ? EXIT_SUCCESS : EXIT_FAILURE;
	}
}