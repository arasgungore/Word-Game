# word-game

A C++ project in which you can play the most naive and elegant game show of the screens, Kelime Oyunu (Word Game) on the terminal. The Turkish question pool contains over 7000 questions to help you boost your word power.



## Introduction

<p align="center">
    <img alt="Screenshot" src="https://github.com/arasgungore/word-game/blob/main/Screenshots/title_screen.jpg" width="1000">
</p>

Welcome to the Word Game!


### Rules

- You will try to guess a total of 14 words/phrases according to the definitions given in 4 minutes.
Words/phrases will be presented to you one by one, and respectively they will have 4, 5, 6, 7, 8, 9, and 10 letters for each pair.

- You will earn 100 points per letter from the words/phrases you guess correctly from these 14 words/phrases.
(a total of 98 letters, which makes the highest possible score 9800)

- For help, you can reveal a letter of current the word/phrase by pressing the 'H' key; but you cannot get points from the revealed letters.
You can stop the timer by pressing the 'B' key (Benjamin), but you cannot reveal any more letters after stopping the timer.

- After the timer stops, you will have 30 seconds to guess the current word/phrase. If you cannot guess the word/phrase correctly within this time,
you will be penalized 100 points per unrevealed letter of the word/phrase.

- You can press the 'P' key to pause and the 'Escape' key to exit the game at any given time.

Press any key to start the game.



## Run on terminal

```sh
g++ word_game.cpp main.cpp -std=c++11 -o test
test
```



## Gameplay

<p align="center">
    <img alt="Screenshot" src="https://github.com/arasgungore/word-game/blob/main/Screenshots/gameplay.gif" width="1000">
</p>



## Author

👤 **Aras Güngöre**

* LinkedIn: [@arasgungore](https://www.linkedin.com/in/arasgungore)
* GitHub: [@arasgungore](https://github.com/arasgungore)
