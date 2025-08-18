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
			starty += fnt.rect.y;
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

	mem->state->animatet = 0;
	mem->state->animation_index = Body_Template_Idle;
	mem->state->animation = animations[mem->state->animation_index];
	mem->state->animate_index = animations[mem->state->animation_index].first_frame;

	mem->state->pos = {};
}



UPDATE_FUNC(game_update)
{
    if(!m->is_init) {
		init_game(m);
    	log_info("gamecode loaded\n");
    }
	GameState *lstate = m->state;

    draw_rectangle(m->sb, {100, 100, 200, 200}, HV_GREEN);

    draw_text(m->sb, {300, 300}, str8(m->transient, "Test"));

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

	ControllerInput *kinput = &m->input->cinput[HV_Keyboard];

	if(kinput->move_right.ended_down) {
		lstate->pos.x += 120.f * m->dt;
		m->state->animation_index = Body_Template_Run;
		anim_helper(&lstate->animate_index, lstate->animation_index);
	} else if(kinput->move_left.ended_down) {
		lstate->pos.x -= 120.f * m->dt;
		m->state->animation_index = Body_Template_Walk;
		anim_helper(&lstate->animate_index, lstate->animation_index);
	}

	if(kinput->move_up.ended_down) {
		lstate->pos.y -= 120.f * m->dt;
	} else if (kinput->move_down.ended_down) {
		lstate->pos.y += 120.0f * m->dt;
	}

	if(was_pressed(&kinput->action_up)) {
		m->state->animation_index = Body_Template_Flat_Jump;
		anim_helper(&lstate->animate_index, lstate->animation_index);
	}

    draw_sprite(m->sb, {lstate->pos.x, lstate->pos.y, 32, 32},
    			(TextureName)lstate->animate_index);


	draw_rectangle(m->sb, {800, 500, 100, 100}, HV_RED);
	draw_rectangle(m->sb, {900, 500, 100, 100}, HV_BLUE);


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
}