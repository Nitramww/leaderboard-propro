# Leaderboard Website
Group project for ProPro 2024.
## Expected Features:
- [x] Basic HTML + CSS website
- [ ] C module that processes data
- [ ] Processed data is shown on the website

## TODO
- [ ] Clean up HTML and CSS code
- [x] Add an "Add event" button

## State of the C Module
- [x] Processes the data coming through a POST request
- [x] Saves the uploaded .txt file's name
- [x] Writes the content of uploaded file to a temporary text file temp.txt
- [x] Reads the data from temp.txt and extracts participants' names, surenames and scores
- [x] Sorts participants from that particular event by their score
- [x] Appends the log.txt file with the uploaded file's name and nicely structured and sorted participants' data
- [ ] Reads the data from the log.txt file and creates an html with leaderboards for EVERY .txt file that can be found in log.txt
