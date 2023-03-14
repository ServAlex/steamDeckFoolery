import pygame
import aiohttp
import asyncio

pygame.joystick.init()
joysticks = [pygame.joystick.Joystick(i) for i in range(pygame.joystick.get_count())]
print(joysticks)
print(joysticks.__len__())
if(joysticks.__len__() > 0):
	for i in range(joysticks.__len__()):
		axixCount = joysticks[i].get_numaxes()
		print(axixCount)
		for j in range(axixCount):
			print(joysticks[i].get_axis(j))

print("main part")

pygame.init()
window = pygame.display.set_mode((1280, 800))
clock = pygame.time.Clock()

rect = pygame.Rect(0, 0, 20, 20)
rect.center = window.get_rect().center
vel = 5

radius = 100

ip = "http://192.168.100.186/"



currentButtonStates = [False, False, False, False]

async def aioRequest(command):
	async with aiohttp.ClientSession(timeout=1) as session:
		try:
			async with session.get(ip + command):
				pass
				#print(resp.status)
				#print(await resp.text())
		except Exception as e:
			print('Connection Error', str(e))

def sendCommand(command):
	print(command)
	asyncio.run(aioRequest(command))


def sendSomeCommand(buttonStates):
	global currentButtonStates

	if(buttonStates == currentButtonStates):
		return

	currentButtonStates = buttonStates
	match buttonStates:
		case [True, False, False, False]: 
			sendCommand("forward")
		case [True, True, False, False]: 
			sendCommand("right")
		case [True, False, False, True]: 
			sendCommand("left")
		case [False, True, False, False]: 
			sendCommand("rotateRight")
		case [False, False, False, True]: 
			sendCommand("rotateLeft")
		case [False, False, False, False]: 
			sendCommand("stop")
		case [False, False, True, False]: 
			sendCommand("reverse")
		case [False, True, True, False]: 
			sendCommand("reverseRight")
		case [False, False, True, True]: 
			sendCommand("reverseLeft")
		case [_,_,_,_]:
			sendCommand("stop")

run = True
while run:
	clock.tick(60)
	for event in pygame.event.get():
		if event.type == pygame.QUIT:
			run = False
		if event.type == pygame.KEYDOWN:
			print(pygame.key.name(event.key))

	keys = pygame.key.get_pressed()

	sendSomeCommand([keys[pygame.K_UP], keys[pygame.K_RIGHT], keys[pygame.K_DOWN], keys[pygame.K_LEFT]])

	if(joysticks.__len__() > 0):
		for i in range(joysticks.__len__()):
			axixCount = joysticks[i].get_numaxes()
			print(axixCount)
			for j in range(axixCount):
				print(joysticks[i].get_axis(j))
	
	"""
	rect.x += (keys[pygame.K_RIGHT] - keys[pygame.K_LEFT]) * vel
	rect.y += (keys[pygame.K_DOWN] - keys[pygame.K_UP]) * vel
	"""

	rect.centerx = window.get_width()/2 + (keys[pygame.K_RIGHT] - keys[pygame.K_LEFT]) * radius
	rect.centery = window.get_height()/2 + (keys[pygame.K_DOWN] - keys[pygame.K_UP]) * radius
		
	rect.centerx = rect.centerx % window.get_width()
	rect.centery = rect.centery % window.get_height()

	window.fill(0)
	pygame.draw.rect(window, (255, 0, 0), rect)
	pygame.display.flip()

pygame.quit()
exit()