import pygame
import aiohttp
import asyncio

current_button_states = [False, False, False, False]
lastUpdated = 0
mode = 0


ip = "ws://192.168.100.186/ws"

async def main(loop):

	async with aiohttp.ClientSession() as session:
		async with session.ws_connect(url = ip) as ws:

			global mode

			pygame.joystick.init()
			joysticks = [pygame.joystick.Joystick(i) for i in range(pygame.joystick.get_count())]
			joysticks_count = joysticks.__len__()

			print("main part")

			pygame.init()
			clock = pygame.time.Clock()

			window = pygame.display.set_mode((1280, 800))
			pygame.display.set_caption("rc controller")


			left_joystick_rect = pygame.Rect(0, 0, 20, 20)
			right_joystick_rect = pygame.Rect(0, 0, 20, 20)
			left_trigger_rect = pygame.Rect(0, 0, 20, 20)
			right_trigger_rect = pygame.Rect(0, 0, 20, 20)
			kb_rect = pygame.Rect(0, 0, 20, 20)
			radius = 100
			joystick_offset = radius * 1.5

			white = (255,255,255)
			text_bg = (10,10,10)

			font = pygame.font.Font('freesansbold.ttf', 16)

			jcount_surface = font.render('joysticks count ' + str(joysticks_count), True, white, text_bg)
			jcount_rect = jcount_surface.get_rect()
			jcount_rect.topleft = (10, 10)

			def joystick_sammary(joystick):
				stats = [str(int(joystick.get_axis(i)*100)) for i in range(joystick.get_numaxes())]
				return ("(" + ", ".join(stats) + ")")

			def draw_joystick_stats():
				joystick_surfaces = [font.render("joystick " + str(i) + ' ' + joystick_sammary(joysticks[i]), True, white, text_bg) for i in range(joysticks_count)]
				joystick_rects = [s.get_rect() for s in joystick_surfaces]
				for i, r in enumerate(joystick_rects):
					r.topleft = (10, 30 + 30*i)
					window.blit(joystick_surfaces[i], joystick_rects[i])

			def draw_line(x, y, text):
				text_surface = font.render(text, True, white, text_bg)
				text_rect = text_surface.get_rect()
				text_rect.topleft = (x, y)
				window.blit(text_surface, text_rect)

			def get_joystick_stats():
				if(joysticks_count < 1):
					return [0, 0, 0, 0, 0, 0]
				return [int(joysticks[0].get_axis(i)*100) for i in range(joysticks[0].get_numaxes())]
				

			lastArray = []

			async def sendWsData(kb, joystick_states):
				array = [kb] + joystick_states

				print("sending ws data")
				#ws.send(bytearray(array))

				await ws.send_bytes(bytearray(array))
				'''
				async for msg in ws:
					if msg.type == aiohttp.WSMsgType.TEXT:
						if msg.data == 'close cmd':
							await ws.close()
							break
						else:
							await ws.send_str(msg.data + '/answer')
				'''

				print("ws data sent")

			async def sendSomeCommand(button_states, joystick_states, mode, time):
				global current_button_states
				global lastUpdated

				current_button_states = button_states

				bytes_to_send = bytearray(button_states)

				for state in joystick_states:
					bytes_to_send.append(state + 128)
				
				bytes_to_send.append(mode)

				print(bytes_to_send, time)

				if(time - lastUpdated > 20):
					await ws.send_bytes(bytes_to_send)
					lastUpdated = time


			run = True
			while run:
				clock.tick(60)
				for event in pygame.event.get():
					if event.type == pygame.QUIT:
						run = False
					if event.type == pygame.KEYDOWN:
						print("keydown " + pygame.key.name(event.key))

						if(event.key == pygame.K_SPACE):
							mode += 1
						if(event.key == pygame.K_COMMA):
							mode -= 1
						if(event.key == pygame.K_PERIOD):
							mode += 1

						if(mode < 0):
							mode = 100
						if(mode > 100):
							mode = 0 

				keys = pygame.key.get_pressed()

				if(keys[pygame.K_q]):
					run = False

				joystick_stats = get_joystick_stats()

				await sendSomeCommand([keys[pygame.K_UP], keys[pygame.K_RIGHT], keys[pygame.K_DOWN], keys[pygame.K_LEFT]], joystick_stats, mode, pygame.time.get_ticks())

				kb_rect.centerx = window.get_width()/2 + (keys[pygame.K_RIGHT] - keys[pygame.K_LEFT]) * radius
				kb_rect.centery = window.get_height()/2 + (keys[pygame.K_DOWN] - keys[pygame.K_UP]) * radius

				left_joystick_rect.centerx = window.get_width()/2 - joystick_offset*2 + joystick_stats[0]
				left_joystick_rect.centery = window.get_height()/2 + joystick_stats[1]

				right_joystick_rect.centerx = window.get_width()/2 + joystick_offset*2 + joystick_stats[3]
				right_joystick_rect.centery = window.get_height()/2 + joystick_stats[4]

				left_trigger_rect.centerx = window.get_width()/2 - joystick_offset*3
				left_trigger_rect.centery = window.get_height()/2 - joystick_stats[2]

				right_trigger_rect.centerx = window.get_width()/2 + joystick_offset*3
				right_trigger_rect.centery = window.get_height()/2 - joystick_stats[5]
					
				window.fill(0)
				pygame.draw.rect(window, (0, 0, 255), kb_rect)
				pygame.draw.rect(window, (0, 255, 0), right_joystick_rect)
				pygame.draw.rect(window, (0, 255, 0), left_joystick_rect)
				pygame.draw.rect(window, (255, 0, 0), left_trigger_rect)
				pygame.draw.rect(window, (255, 0, 0), right_trigger_rect)
				window.blit(jcount_surface, jcount_rect)
				draw_joystick_stats()
				draw_line(10, 60, "Mode: " + str(mode))
				pygame.display.flip()

				await asyncio.sleep(0)

			pygame.quit()
			exit()


loop = asyncio.get_event_loop()
loop.run_until_complete(main(loop))