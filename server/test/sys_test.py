import subprocess
import smtplib
import os

subp = subprocess.Popen("./notsmtp")

sender = 'from@fromdomain.com'
receivers = ['to@todomain.com']
message = """From: From Person <from@fromdomain.com>
To: To Person <to@todomain.com>
Subject: SMTP e-mail test

This is a test e-mail message.
"""

send = 0
while (send == 0):
    try:
        smtpObj = smtplib.SMTP('localhost', 2525, local_hostname="localhost")
        smtpObj.sendmail(sender, receivers, message)         
        print("[SYSTEM TEST] SUCCESSFULLY SENT EMAIL")
        send = 1
    except Exception:
        print("Error: unable to send email")

subp.kill()
