import requests

headers = {

}
for i in range(10):
    req = requests.get("https://127.0.0.1:8888",
                       headers=headers,
                       verify=False,
                       )

    print(req.content.decode('utf8'))
