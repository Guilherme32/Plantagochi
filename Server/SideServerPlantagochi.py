from fastapi import FastAPI, Request
from pydantic import BaseModel
from datetime import datetime
import sqlite3
import os
from fastapi.responses import HTMLResponse
from fastapi.templating import Jinja2Templates

class Data(BaseModel):
    temperature: int
    soil_humidity: int
    ambient_humidity: int

app = FastAPI()
templates = Jinja2Templates(directory="templates")

# Route for the home page
@app.get('/')
def home(request: Request):
    background_color = '#f2f2f2'  # Default background color
    return templates.TemplateResponse("home.html", {"request": request, "background_color": background_color})
""" @app.get("/", response_class=HTMLResponse)
async def root():
    # Get the current directory of the file
    current_dir = os.path.dirname(os.path.abspath(__file__))

    # Construct the relative path to the database file
    db_path = os.path.join(current_dir, "../Database/plantagochi.db")

    # Update the database connection code
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM sensor_data")
    rows = cursor.fetchall()
    conn.close()

    if not rows:
        return "<h1>No data available</h1>"

    table_html = "<table style='margin: auto;'>"
    table_html += "<tr><th>Datetime</th><th>Temperature</th><th>Soil Humidity</th><th>Ambient Humidity</th></tr>"
    for row in rows:
        try:
            datetime_str = datetime.strptime(row[1], "%Y-%m-%d %H:%M:%S.%f").strftime("%Y-%m-%d %H:%M:%S")
        except ValueError:
            datetime_str = "Invalid datetime"
        table_html += f"<tr><td style='text-align: center;'>{datetime_str}</td><td style='text-align: center;'>{row[2]}</td><td style='text-align: center;'>{row[3]}</td><td style='text-align: center;'>{row[4]}</td></tr>"
    table_html += "</table>"

    return f'''
        <html>
        <head>
            <title>Sensor Data</title>
            <style>
                h1 {{
                    text-align: center;
                }}
            </style>
        </head>
        <body>
            <h1>Sensor Data</h1>
            {table_html}
        </body>
        </html>
    ''' """

@app.route('/database')
def database():
    # Retrieve your database data here and pass it to the template
    # Example data
    database_data = [
        {'datetime': 'datadb'},
        {'temperature': 'tempdb'},
        {'soilhumidity': 'shdb'},
        {'ambienthumidity': 'ahdb'}
    ]
    return render_template('database.html', data=database_data)

@app.get("/PlantagochiUFJF_GGJJL/")
async def root():
    return {"id": "Plantagochi!!"}

@app.post("/data/")
async def root(data: Data):


    # Get the current directory of the file
    current_dir = os.path.dirname(os.path.abspath(__file__))

    # Construct the relative path to the database file
    db_path = os.path.join(current_dir, "../Database/plantagochi.db")

    # Update the database connection code
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute(
        "CREATE TABLE IF NOT EXISTS sensor_data (id INTEGER PRIMARY KEY AUTOINCREMENT, datetime DATETIME, temperature INT, soil_humidity INT, ambient_humidity INT)"
    )
    cursor.execute(
        "INSERT INTO sensor_data (datetime, temperature, soil_humidity, ambient_humidity) VALUES (?, ?, ?, ?)",
        (datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"), data.temperature, data.soil_humidity, data.ambient_humidity),
    )
    conn.commit()
    conn.close()

    print(f"Data received: {data}")
    return {"Done": "OK"}
