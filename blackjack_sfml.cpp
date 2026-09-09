#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <chrono>
using namespace std;

// ---------------- Card ----------------
class Card {
public:
    string suit;
    string value;

    Card() {}
    Card(string s, string v) : suit(s), value(v) {}

    int getBlackjackValue() const {
        if (value == "Ace") return 11;
        if (value == "Jack" || value == "Queen" || value == "King") return 10;
        return stoi(value);
    }

    string toString() const {
        return value + " of " + suit;
    }

    // Builds the exact filename Kenney's pack uses, e.g. "card_hearts_A.png"
    string getImageFilename() const {
        string suitLower;
        if (suit == "Hearts") suitLower = "hearts";
        else if (suit == "Diamonds") suitLower = "diamonds";
        else if (suit == "Clubs") suitLower = "clubs";
        else suitLower = "spades";

        string valueCode;
        if (value == "Ace") valueCode = "A";
        else if (value == "Jack") valueCode = "J";
        else if (value == "Queen") valueCode = "Q";
        else if (value == "King") valueCode = "K";
        else {
            int v = stoi(value);
            valueCode = (v < 10 ? "0" : "") + to_string(v);
        }

        return "card_" + suitLower + "_" + valueCode + ".png";
    }
};

// ---------------- Deck ----------------
class Deck {
public:
    vector<Card> cards;

    Deck() {
        string suits[4] = {"Hearts", "Diamonds", "Clubs", "Spades"};
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j <= 13; j++) {
                string val;
                if (j == 1) val = "Ace";
                else if (j == 11) val = "Jack";
                else if (j == 12) val = "Queen";
                else if (j == 13) val = "King";
                else val = to_string(j);
                cards.push_back(Card(suits[i], val));
            }
        }
    }

    void shuffle() {
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(cards.begin(), cards.end(), std::default_random_engine(seed));
    }

    Card dealCard() {
        Card top = cards.back();
        cards.pop_back();
        return top;
    }
};

// ---------------- Player ----------------
class Player {
public:
    string name;
    int money;
    vector<Card> hand;

    Player(string n, int startingMoney = 100) : name(n), money(startingMoney) {}

    void addCard(const Card& c) { hand.push_back(c); }
    void clearHand() { hand.clear(); }

    int getHandValue() const {
        int total = 0;
        int aceCount = 0;
        for (const auto& c : hand) {
            total += c.getBlackjackValue();
            if (c.value == "Ace") aceCount++;
        }
        while (total > 21 && aceCount > 0) {
            total -= 10;
            aceCount--;
        }
        return total;
    }

    bool isBust() const { return getHandValue() > 21; }
    bool hasBlackjack() const { return hand.size() == 2 && getHandValue() == 21; }
};

// ---------------- TextureManager ----------------
// Loads every card texture ONCE at startup, keyed by filename.
// This is the piece that connects your Card objects to actual images.
class TextureManager {
public:
    map<string, sf::Texture> textures;
    string basePath;

    bool loadAll(const string& path) {
        basePath = path;
        string suits[4] = {"hearts", "diamonds", "clubs", "spades"};
        vector<string> values = {"A", "02", "03", "04", "05", "06", "07",
                                  "08", "09", "10", "J", "Q", "K"};

        bool allOk = true;

        for (const string& suit : suits) {
            for (const string& val : values) {
                string filename = "card_" + suit + "_" + val + ".png";
                sf::Texture tex;
                if (!tex.loadFromFile(basePath + filename)) {
                    cout << "FAILED to load: " << basePath + filename << endl;
                    allOk = false;
                } else {
                    textures[filename] = tex;
                }
            }
        }

        // Card back, for the dealer's hidden card
        sf::Texture backTex;
        if (!backTex.loadFromFile(basePath + "card_back.png")) {
            cout << "FAILED to load: " << basePath + "card_back.png" << endl;
            allOk = false;
        } else {
            textures["card_back.png"] = backTex;
        }

        return allOk;
    }

    // Returns a reference to the loaded texture for a given card
    const sf::Texture& get(const Card& card) const {
        return textures.at(card.getImageFilename());
    }

    const sf::Texture& getBack() const {
        return textures.at("card_back.png");
    }
};

// ---------------- Renderer ----------------
// Knows how to draw hands on screen. Reads game state; never changes it.
class Renderer {
public:
    TextureManager& textures;
    float cardScale = 3.0f; // Kenney's cards are large; scale them down

    Renderer(TextureManager& tm) : textures(tm) {}

    void drawHand(sf::RenderWindow& window, const vector<Card>& hand,
                  float startX, float startY, bool hideFirst = false) {
        float xOffset = 0.0f;
        float cardSpacing = 90.0f; // horizontal gap between cards

        for (size_t i = 0; i < hand.size(); i++) {
            sf::Sprite sprite(hideFirst && i == 0
                                   ? textures.getBack()
                                   : textures.get(hand[i]));

            sprite.setScale({cardScale, cardScale});
            sprite.setPosition({startX + xOffset, startY});
            window.draw(sprite);

            xOffset += cardSpacing;
        }
    }
};

// ---------------- Game ----------------
class Game {
public:
    Deck deck;
    Player player;
    Player dealer;
    int bet = 10;
    bool roundOver = false;
    bool playerBust = false;
    string resultMessage = "";

    Game() : player("You", 100), dealer("Dealer", 0) {}

    void startRound() {
        deck = Deck();
        deck.shuffle();
        player.clearHand();
        dealer.clearHand();
        roundOver = false;
        playerBust = false;
        resultMessage = "";

        player.addCard(deck.dealCard());
        dealer.addCard(deck.dealCard());
        player.addCard(deck.dealCard());
        dealer.addCard(deck.dealCard());

        if (player.hasBlackjack()) {
            resultMessage = "Blackjack! You win!";
            player.money += static_cast<int>(bet * 1.5);
            roundOver = true;
        }
    }

    void playerHit() {
        if (roundOver) return;
        player.addCard(deck.dealCard());
        if (player.isBust()) {
            playerBust = true;
            roundOver = true;
            resultMessage = "You busted! Dealer wins.";
            player.money -= bet;
        }
    }

    void playerStand() {
        if (roundOver) return;

        while (dealer.getHandValue() < 17) {
            dealer.addCard(deck.dealCard());
        }

        int playerVal = player.getHandValue();
        int dealerVal = dealer.getHandValue();

        if (dealer.isBust()) {
            resultMessage = "Dealer busts! You win!";
            player.money += bet;
        } else if (playerVal > dealerVal) {
            resultMessage = "You win!";
            player.money += bet;
        } else if (playerVal < dealerVal) {
            resultMessage = "Dealer wins.";
            player.money -= bet;
        } else {
            resultMessage = "Push (tie).";
        }
        roundOver = true;
    }
};

// ---------------- Simple clickable button ----------------
struct Button {
    sf::RectangleShape shape;
    sf::Text label;

    Button(sf::Font& font, const string& text, sf::Vector2f pos, sf::Vector2f size) 
        : label(font, text, 20) {
        shape.setSize(size);
        shape.setPosition(pos);
        shape.setFillColor(sf::Color(60, 60, 60));
        shape.setOutlineColor(sf::Color::White);
        shape.setOutlineThickness(2.0f);

        label.setFillColor(sf::Color::White);
        // Roughly center the text in the button
        label.setPosition({pos.x + size.x / 2.0f - label.getLocalBounds().size.x / 2.0f,
                            pos.y + size.y / 2.0f - label.getLocalBounds().size.y / 2.0f - 5});
    }

    bool isClicked(sf::Vector2i mousePos) const {
        return shape.getGlobalBounds().contains(sf::Vector2f(mousePos));
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
        window.draw(label);
    }
};

// ---------------- Main ----------------
int main() {
    sf::RenderWindow window(sf::VideoMode({1000, 700}), "Blackjack");
    window.setFramerateLimit(60);

    // Load card textures - adjust this path to match your actual folder structure
    TextureManager textures;
    if (!textures.loadAll("assets/PNG/Cards (large)/")) {
        cout << "\nSome textures failed to load. Check the path above and your assets folder.\n";
        cout << "Expected structure: assets/PNG/card_hearts_A.png, etc.\n";
        return 1;
    }

    // Load a font for text rendering (Hit/Stand button labels, hand values)
    sf::Font font;
    if (!font.openFromFile("assets/BrownieStencil-8O8MJ.ttf")) {
        cout << "\nCould not load assets/BrownieStencil-8O8MJ.ttf\n";
        cout << "Download any free .ttf font and place it there (e.g. from Google Fonts).\n";
        return 1;
    }

    Renderer renderer(textures);
    Game game;
    game.startRound();

    Button hitButton(font, "Hit", {750, 550}, {100, 50});
    Button standButton(font, "Stand", {870, 550}, {100, 50});
    Button newRoundButton(font, "New Round", {750, 620}, {220, 50});

    sf::Text infoText(font, "", 24);
    infoText.setFillColor(sf::Color::White);
    infoText.setPosition({50, 20});

    // Dedicated banner for the round result — kept in its own fixed spot
    // (top-center, above everything) so it's never covered by cards,
    // no matter how big cardScale gets.
    sf::Text resultText(font, "", 32);
    resultText.setFillColor(sf::Color::Yellow);
    resultText.setStyle(sf::Text::Bold);

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button == sf::Mouse::Button::Left) {
                    sf::Vector2i mousePos = {mousePressed->position.x, mousePressed->position.y};

                    if (hitButton.isClicked(mousePos)) {
                        game.playerHit();
                    } else if (standButton.isClicked(mousePos)) {
                        game.playerStand();
                    } else if (newRoundButton.isClicked(mousePos)) {
                        game.startRound();
                    }
                }
            }
        }

        window.clear(sf::Color(20, 90, 50)); // card table green

        // Dealer's hand (hide first card until round is over)
        renderer.drawHand(window, game.dealer.hand, 50, 80, !game.roundOver);

        // Player's hand
        renderer.drawHand(window, game.player.hand, 50, 380);

        // Info text: money and your own hand value only.
        // (Dealer's hand value now lives in the result banner below,
        // so it's never at risk of being covered by enlarged cards.)
        string info = "Money: $" + to_string(game.player.money) + "\n";
        info += "Your hand: " + to_string(game.player.getHandValue());
        infoText.setString(info);
        window.draw(infoText);

        hitButton.draw(window);
        standButton.draw(window);
        newRoundButton.draw(window);

        // Draw the result banner LAST so it renders on top of everything,
        // including cards, if there's ever any overlap.
        if (game.roundOver && !game.resultMessage.empty()) {
            string banner = "Dealer: " + to_string(game.dealer.getHandValue())
                             + "   |   " + game.resultMessage;
            resultText.setString(banner);
            // Center it horizontally based on the window width and the text's own width
            float textWidth = resultText.getLocalBounds().size.x;
            float windowWidth = static_cast<float>(window.getSize().x);
            resultText.setPosition({(windowWidth - textWidth) / 2.0f, 15.0f});

            // A dark semi-transparent backing box behind the text improves
            // readability against card table green or light card backs.
            sf::RectangleShape backing({textWidth + 40.0f, 50.0f});
            backing.setPosition({(windowWidth - textWidth) / 2.0f - 20.0f, 5.0f});
            backing.setFillColor(sf::Color(0, 0, 0, 180));
            window.draw(backing);
            window.draw(resultText);
        }

        window.display();
    }

    return 0;
}