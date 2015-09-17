# CacheBot README.md
## Conversation flows
### Prompts
Prompts are notifications sent by Cache, that may or may not include a question (preferably yes/no)
#### Income prompts

### Yes/no buying decisions
### Advice
#### Types
##### Price windows
```
{
    type         : String (e.g. 'window')
    , score      : Number (the average daily difference divded by the time we are noticing this difference over = $/day/day)
                          (It is like amortizing the bad/good behavior)
    , approxAmnt : Number (e.g. 50)
    , factStr    : String (temporary human readable version for debugging)
    , dailyOld   : Number ($/day)
    , dailyNew   : Number ($/day)
    , dailyDif   : Number ($/day)
    , flowEffect : Number (flow change)
    , overDays   : Number (e.g. 7 for a trend7)
}
```
