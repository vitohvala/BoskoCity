#include "game.h"

/*
	TODO: Animations
*/

function void
hv_append(SpriteBatch *sb, Vec4 sprpos, Vec4 atlpos, Vec3 color)
{
    sb->sprite[sb->count].pos = 	{ sprpos.x, sprpos.y };
    sb->sprite[sb->count].size = 	{ sprpos.z, sprpos.w };
    sb->sprite[sb->count].aoffset = {atlpos.x, atlpos.y};
    sb->sprite[sb->count].asize = 	{atlpos.z, atlpos.w};
    sb->sprite[sb->count].color = 	color;
    sb->count++;
}

function void
draw_text(SpriteBatch *sb, Vec2 pos, String8 text, Vec3 color = vec3(1.0f))
{
	f32 startx = pos.x;
	f32 starty = pos.y;

	for (usize i = 0; i < text.len; ++i) {
		Glyph fnt = glyphs[0];
		u8 letter = text[i];

		fnt = glyphs[letter - 32];

		if(letter == '\n') {
			starty += fnt.rect.y + (f32)fnt.offset_y;
			startx = pos.x;
			continue;
		}

		f32 startx1 = startx + (f32)fnt.offset_x;
		f32 starty1 = starty + (f32)fnt.offset_y;

		f32 endx = fnt.rect.z;
		f32 endy = fnt.rect.w;

		hv_append(sb, (Vec4){startx1, starty1, endx, endy}, fnt.rect, color);

		startx += (f32)fnt.advance_x;

	}
}

function inline void
draw_rectangle(SpriteBatch *sb, Vec4 pos, Vec3 color = vec3(1.0f)) {
    hv_append(sb, pos, glyphs['.' - 32].rect, color);
}

function inline void
draw_sprite(SpriteBatch *sb, Vec4 possize, TextureName tname,Vec3 color = vec3(1.0f))
{
	hv_append(sb, possize, textures[tname].rect, color);
}

function inline void
draw_sprite_flip_x(SpriteBatch *sb, Vec4 possize, TextureName tname,  Vec3 color = vec3(1.0f))
{
	Vec4 trect = textures[tname].rect;
	trect.x = trect.x + trect.z;
	trect.z = -trect.z;
	//draw_sprite_flip_x(m->sb, {lstate->pos.x, lstate->pos.y, 32, 32}, trect);
	hv_append(sb, possize, trect, color);
}

function inline void
draw_sprite_v2(SpriteBatch *sb, Vec2 pos, TextureName tname,Vec3 color = vec3(1.0f))
{
	Vec4 sprite = textures[tname].rect;
	Vec4 sb_pos = {pos.x, pos.y, sprite.z, sprite.w};
	hv_append(sb, sb_pos, sprite, color);
}

function inline void
anim_helper(usize *animate_index, usize animation_index)
{
		if (*animate_index < animations[animation_index].first_frame
		   || *animate_index > animations[animation_index].last_frame)
		{
			*animate_index = animations[animation_index].first_frame;
		}
}

function void
init_game(Memory *mem)
{
	set_console(mem->log);
	mem->is_init = true;
	mem->state = (GameState*)arena_alloc(mem->permanent, sizeof(GameState));

	mem->state->pos = vec2(75.0f);
	mem->state->pos.x = 64.0f;

	mem->state->vel = vec2(0.0f);

	mem->state->animatet = 0;
	mem->state->animation_index = Body_Template_Idle;
	mem->state->animation = animations[mem->state->animation_index];
	mem->state->animate_index = animations[mem->state->animation_index].first_frame;

}

function void
handle_input(GameState *state, ControllerInput *kinput, f32 dt)
{
	usize anim__ = Body_Template_Idle;
	if(kinput->move_right.ended_down) {
		state->vel.x += 240.f;
		anim__ = Body_Template_Run;
		state->flip = false;

	} else if(kinput->move_left.ended_down) {
		state->vel.x -= 240.f;
		anim__ = Body_Template_Run;
		state->flip = true;
	}

	if(kinput->move_down.ended_down) {
		state->vel.y = 120.f;
	} else if(kinput->move_up.ended_down) {
		state->vel.y = -120.f;
	}

	if(was_pressed(&kinput->action_up)) {
		state->vel.y = -800.f;
		state->animation_index = Body_Template_Jump;
		anim_helper(&state->animate_index, state->animation_index);
	}

	if (state->animation_index != Body_Template_Jump) {
		state->animation_index = anim__;
		anim_helper(&state->animate_index, state->animation_index);
	}
}

int startx = 0;
int starty = 0;

UPDATE_FUNC(game_update)
{
	if(m->is_init == 2) {
		set_console(m->log);
		m->is_init = true;
	}
    if(!m->is_init) {
		init_game(m);
    	log_info("gamecode loaded\n");
    	m->state->level = (u32 *)game_level5;
    }

	GameState *lstate = m->state;
	ControllerInput *kinput = &m->input->cinput[HV_Keyboard];

	lstate->vel.y += 2000 * m->dt;
	handle_input(lstate, kinput, m->dt);

	//lstate->pos.y += lstate->vel.y * m->dt;

	lstate->animatet += m->dt;
	lstate->animation = animations[lstate->animation_index];

	if(lstate->animatet > textures[lstate->animate_index].duration ) {
		lstate->animate_index++;
		lstate->animatet = 0.0f;
		if(lstate->animate_index > lstate->animation.last_frame){
			m->state->animation_index = Body_Template_Idle;
			anim_helper(&lstate->animate_index, lstate->animation_index);
			//lstate->animate_index = lstate->animation.first_frame;
		}
	}

	//TODO: Reset dugme
	if (was_pressed(&kinput->start)) {
		lstate->pos = {65.0f * 10.f, 65.0f + 8.0f};
		lstate->vel = vec2(0.0f);
	}

	Vec2 nextpos = lstate->pos + (lstate->vel * m->dt);

	int mnext_posx = CLAMP(((int)nextpos.x + 32) / TILE_SIZE + startx, 0, GAME_LEVEL_WIDTH - 1);
	int mnext_posw = CLAMP(((int)nextpos.x)      / TILE_SIZE + startx, 0, GAME_LEVEL_WIDTH - 1);
	int posx = 		 CLAMP(((int)lstate->pos.x)  / TILE_SIZE + startx, 0, GAME_LEVEL_WIDTH - 1);

	int mnext_posy = CLAMP((((int)nextpos.y + 32)) / TILE_SIZE + starty, 0, GAME_LEVEL_HEIGHT - 1);
	int mnext_posh = CLAMP((((int)nextpos.y))      / TILE_SIZE + starty, 0, GAME_LEVEL_HEIGHT - 1);
	int posy = 		 CLAMP(((int)lstate->pos.y)    / TILE_SIZE + starty, 0, GAME_LEVEL_HEIGHT - 1);

	draw_text(m->sb, {0, 0}, tmp_format_str8(m->transient, "posx  %d posy  %d", posx, posy), HV_RED);

	if (lstate->level[mnext_posy *  GAME_LEVEL_WIDTH + mnext_posx] == 1 ||
		lstate->level[mnext_posh *  GAME_LEVEL_WIDTH + mnext_posx] == 1 ||
		lstate->level[mnext_posy *  GAME_LEVEL_WIDTH + mnext_posw] == 1 ||
		lstate->level[mnext_posh *  GAME_LEVEL_WIDTH + mnext_posw] == 1)  {
		lstate->vel.y = 0;
		//lstate->pos.y = nextpos.y;
		nextpos.y = lstate->pos.y;
	}

	if(lstate->level[posy * GAME_LEVEL_WIDTH + mnext_posx] == 1 || lstate->level[posy * GAME_LEVEL_WIDTH + mnext_posw] == 1) {
		//lstatworld->level.x = 0;
		//lstate->pos.x = nextpos.x;
		nextpos.x = lstate->pos.x;
	}

	lstate->pos = nextpos;

	if(lstate->level[posy * GAME_LEVEL_WIDTH + mnext_posw] == 2) {
		if(lstate->pos.x > 1280)   { startx = 20; lstate->pos.x = 0.0f; }
		else if(lstate->pos.x < 0) { startx = 0; lstate->pos.x = 1280.0f; }

		if(lstate->pos.y > 720)     { starty = 11; lstate->pos.y = 0.0f; }
		else if (lstate->pos.y < 0) { starty = 0;  lstate->pos.y = 720.0f; }
	}


	lstate->vel.x = 0.0f;
	//if(lstate->animation_index != Body_Template_Flat_Jump) lstate->vel.y = 0;

	#if defined(false)
	String8 text_dt;
	text_dt.data = (u8*)arena_alloc(m->transient, 2048);
	text_dt.len = (usize)stbsp_snprintf((char*)text_dt.data, 2047, "dt = %.2f",
									    m->dt);
    draw_text(m->sb, {1100, 10}, text_dt);

	usize mem_fmt = 1;
	if(m->transient->curr_offset < Megabyte) mem_fmt = Kilobyte;
	else if(m->transient->curr_offset < Gigabyte) mem_fmt = Megabyte;
	else mem_fmt = Gigabyte;

	String8 buffer;
	buffer.data = (u8*)arena_alloc(m->transient, 2048);
	buffer.len = (usize)stbsp_snprintf((char*)buffer.data, 2047, "Memory per frame %dkb",
									  (m->transient->curr_offset) / mem_fmt);
    draw_text(m->sb, {0, 690}, buffer);
    #endif


	for(int y_osa = 0; y_osa < LEVEL_WIDTH; ++y_osa) {
		for(int x_osa = 0; x_osa < LEVEL_WIDTH; ++x_osa) {
			Vec3 color = vec3(1.0f);
			int val = lstate->level[(y_osa + starty) * GAME_LEVEL_WIDTH + x_osa + startx];
			f32 __x = ((f32)x_osa * (f32)TILE_SIZE);
			f32 __y = ((f32)y_osa * (f32)TILE_SIZE);
			Vec4 draw_rect = {__x, __y, (f32)TILE_SIZE, (f32)TILE_SIZE};

			if(val == 0) {
				draw_rectangle(m->sb, draw_rect, {((f32)x_osa + 20) / 255, ((f32)x_osa + 20) / 255, ((f32)x_osa + 180) / 255});
			} else if(val == 1) {
				color = HV_RED;
				draw_sprite(m->sb, draw_rect, Wall);
			} else if(val == 2) {
				color = HV_GREEN;
			}

		}
	}


	if(lstate->flip) {
		draw_sprite_flip_x(m->sb, {lstate->pos.x, lstate->pos.y, 32, 32},
							(TextureName)lstate->animate_index);
	} else {
    	draw_sprite(m->sb, {lstate->pos.x, lstate->pos.y, 32, 32},
    			(TextureName)lstate->animate_index);
	}

}

