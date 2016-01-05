/*
* This source file is part of an OSTIS project. For the latest info, see http://ostis.net
* Distributed under the MIT License
* (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
*/

#include "iotKeynodes.hpp"
#include "wrap/sc_memory.hpp"

namespace iot
{
	sc::Addr Keynodes::device;
	sc::Addr Keynodes::device_enabled;
	sc::Addr Keynodes::device_energy_usage;
	sc::Addr Keynodes::device_standby_energy_usage;
	sc::Addr Keynodes::device_real_energy_usage;

	sc::Addr Keynodes::device_group_enable_command;
	sc::Addr Keynodes::device_group_disable_command;
	sc::Addr Keynodes::add_content_command;

	sc::Addr Keynodes::group_volume;
	sc::Addr Keynodes::command_initiated;
	sc::Addr Keynodes::command_finished;
	sc::Addr Keynodes::command_in_progress;

	sc::Addr Keynodes::nrel_content;
	sc::Addr Keynodes::nrel_mass;

	sc::Addr Keynodes::rrel_1;
	sc::Addr Keynodes::rrel_2;
	sc::Addr Keynodes::rrel_3;
	sc::Addr Keynodes::rrel_gram;

	sc::MemoryContext * Keynodes::memory_ctx = 0;

	bool Keynodes::initialize()
	{
		memory_ctx = new sc::MemoryContext(sc_access_lvl_make_min, "IoTKeynodes");
		if (!memory_ctx)
			return false;

		bool result = true;

		result = result && resolveKeynode("device", device);
		result = result && resolveKeynode("device_enabled", device_enabled);
		result = result && resolveKeynode("nrel_energy_usage", device_energy_usage);
		result = result && resolveKeynode("nrel_standby_energy_usage", device_standby_energy_usage);
		result = result && resolveKeynode("nrel_real_energy_usage", device_real_energy_usage);

		result = result && resolveKeynode("device_group_enable_command", device_group_enable_command);
		result = result && resolveKeynode("device_group_disable_command", device_group_disable_command);
		result = result && resolveKeynode("add_content_command", add_content_command);

		result = result && resolveKeynode("group_volume", group_volume);
		result = result && resolveKeynode("command_initiated", command_initiated);
		result = result && resolveKeynode("command_finished", command_finished);
		result = result && resolveKeynode("command_in_progress", command_in_progress);

		result = result && resolveKeynode("nrel_content", nrel_content);
		result = result && resolveKeynode("nrel_mass", nrel_mass);

		result = result && resolveKeynode("rrel_1", rrel_1);
		result = result && resolveKeynode("rrel_2", rrel_2);
		result = result && resolveKeynode("rrel_3", rrel_3);
		result = result && resolveKeynode("rrel_gram", rrel_gram);

		return result;
	}

	bool Keynodes::shutdown()
	{
		if (memory_ctx)
		{
			delete memory_ctx;
			memory_ctx = 0;
		}
		return true;
	}

	bool Keynodes::resolveKeynode(std::string const & sysIdtf, sc::Addr & outAddr)
	{
		check_expr(memory_ctx);

		// check if node with system identifier already exists
		if (memory_ctx->helperResolveSystemIdtf(sysIdtf, outAddr))
			return true;

		// create new node with specified system identifier
		outAddr = memory_ctx->createNode(sc_type_node);
		if (outAddr.isValid())
			return memory_ctx->helperSetSystemIdtf(sysIdtf, outAddr);

		return false;
	}
}