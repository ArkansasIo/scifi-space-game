// src/db/schema.c++ -- the database schema as SQL text.

#include "spacebattlerpg.h"
#include "db/schema.h"

/*
 * Every table is IF NOT EXISTS so dbCreateSchema() is idempotent.
 * Column names mirror the struct fields in include/core/types.h exactly,
 * which keeps the save/load code a straight field-to-column copy.
 */

const char *DB_SCHEMA_SQL =
    "CREATE TABLE IF NOT EXISTS CoreAttributes (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  owner VARCHAR(64) NOT NULL,\n"
    "  str INT, dex INT, con INT, intel INT, wis INT,\n"
    "  vit INT, spi INT, lck INT, wil INT, cha INT,\n"
    "  UNIQUE KEY uq_owner (owner)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS StatCaps (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  name VARCHAR(24) NOT NULL,\n"
    "  pve_cap INT, pvp_cap INT, soft_cap INT, curve_k INT,\n"
    "  UNIQUE KEY uq_name (name)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS Effects (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  name VARCHAR(30) NOT NULL,\n"
    "  category INT NOT NULL,\n"
    "  damage_type INT NOT NULL,\n"
    "  stack_mode INT, max_stacks INT, duration INT,\n"
    "  magnitude INT, tick_rate INT, dr_category INT,\n"
    "  cleanse INT, immunity_window INT,\n"
    "  UNIQUE KEY uq_name (name)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS EnemyArchetypes (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  name VARCHAR(30) NOT NULL,\n"
    "  tier INT NOT NULL,\n"
    "  role INT NOT NULL,\n"
    "  preferred_range INT, aggro_radius INT,\n"
    "  ability_count INT, loot_profile INT, spawn_weight INT,\n"
    "  UNIQUE KEY uq_name (name)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS Affixes (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  name VARCHAR(30) NOT NULL,\n"
    "  category INT NOT NULL,\n"
    "  magnitude INT, stack_cost INT,\n"
    "  UNIQUE KEY uq_name (name)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS Bosses (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  name VARCHAR(30) NOT NULL,\n"
    "  tier INT, level INT, hp_pool INT, phase_count INT,\n"
    "  arena_hazard_count INT, soft_enrage_rate INT,\n"
    "  hard_enrage_time INT, loot_table_index INT,\n"
    "  party_scaling INT, tags VARCHAR(64),\n"
    "  UNIQUE KEY uq_name (name)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS BossPhases (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  boss_id INT NOT NULL,\n"
    "  phase_index INT NOT NULL,\n"
    "  name VARCHAR(30), hp_threshold INT, time_limit INT,\n"
    "  ability_count INT, add_spawner_index INT, immune_mask INT,\n"
    "  KEY idx_boss (boss_id),\n"
    "  CONSTRAINT fk_phase_boss FOREIGN KEY (boss_id)\n"
    "    REFERENCES Bosses(id) ON DELETE CASCADE\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS Items (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  name VARCHAR(30) NOT NULL,\n"
    "  family INT NOT NULL, slot INT NOT NULL, rarity INT NOT NULL,\n"
    "  required_level INT, bind_rule INT,\n"
    "  proc_value_pu INT, set_bonus_value_pu INT,\n"
    "  item_power_pve INT, item_power_pvp INT,\n"
    "  UNIQUE KEY uq_name (name)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS ItemStats (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  item_id INT NOT NULL,\n"
    "  context INT NOT NULL,\n"
    "  line_index INT NOT NULL,\n"
    "  label VARCHAR(24), raw INT, weight INT, value_pu INT,\n"
    "  KEY idx_item (item_id, context),\n"
    "  CONSTRAINT fk_stat_item FOREIGN KEY (item_id)\n"
    "    REFERENCES Items(id) ON DELETE CASCADE\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS GearSets (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  name VARCHAR(30) NOT NULL,\n"
    "  piece_count INT, bonus2 INT, bonus4 INT, bonus6 INT,\n"
    "  UNIQUE KEY uq_name (name)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS LootTables (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  name VARCHAR(30) NOT NULL,\n"
    "  entry_count INT, pity_timer INT, unique_per_week INT,\n"
    "  difficulty_multiplier INT, distribution_rule INT,\n"
    "  UNIQUE KEY uq_name (name)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS LootEntries (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  table_id INT NOT NULL,\n"
    "  entry_index INT NOT NULL,\n"
    "  name VARCHAR(30), rarity INT, weight INT, min_level INT,\n"
    "  KEY idx_table (table_id),\n"
    "  CONSTRAINT fk_entry_table FOREIGN KEY (table_id)\n"
    "    REFERENCES LootTables(id) ON DELETE CASCADE\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS Encounters (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  name VARCHAR(30) NOT NULL,\n"
    "  kind INT NOT NULL, trash_count INT, caster_count INT,\n"
    "  miniboss_count INT, wave_count INT,\n"
    "  has_hazard INT, has_puzzle INT, timed INT, boss_index INT,\n"
    "  UNIQUE KEY uq_name (name)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS Factions (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  name VARCHAR(30) NOT NULL,\n"
    "  industry INT, science INT, economy INT, influence INT,\n"
    "  logistics INT, intelligence INT, stability INT,\n"
    "  credits INT, minerals INT, gas INT, energy INT, data INT,\n"
    "  reputation INT, at_war INT,\n"
    "  UNIQUE KEY uq_name (name)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS StarSystems (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  name VARCHAR(30) NOT NULL,\n"
    "  sector_index INT, planet_count INT,\n"
    "  has_asteroid_field INT, has_station INT, has_anomaly INT,\n"
    "  owner_faction INT,\n"
    "  UNIQUE KEY uq_name (name)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS Planets (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  system_id INT NOT NULL,\n"
    "  planet_index INT NOT NULL,\n"
    "  name VARCHAR(30), size INT, habitability INT,\n"
    "  resources INT, defense INT, stability INT, building_count INT,\n"
    "  KEY idx_system (system_id),\n"
    "  CONSTRAINT fk_planet_system FOREIGN KEY (system_id)\n"
    "    REFERENCES StarSystems(id) ON DELETE CASCADE\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS Ships (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  fleet_id INT NOT NULL,\n"
    "  ship_index INT NOT NULL,\n"
    "  name VARCHAR(30), ship_class INT, hull INT, shields INT,\n"
    "  firepower INT, crew_skill INT, tech_level INT, supply_use INT,\n"
    "  KEY idx_fleet (fleet_id)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS Players (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  name VARCHAR(30) NOT NULL,\n"
    "  power INT, sif INT, sp INT, exp INT, health INT, level INT,\n"
    "  lightlevel INT, attackpower INT, defencepower INT,\n"
    "  weapon INT, armour INT, shield INT,\n"
    "  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP\n"
    "    ON UPDATE CURRENT_TIMESTAMP,\n"
    "  UNIQUE KEY uq_name (name)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS RunScores (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  player VARCHAR(30) NOT NULL,\n"
    "  score INT, actions_taken INT, damage_taken INT,\n"
    "  kills INT, damage_done INT,\n"
    "  recorded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,\n"
    "  KEY idx_player (player)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n"

    "CREATE TABLE IF NOT EXISTS Telemetry (\n"
    "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
    "  event_id INT NOT NULL,\n"
    "  arg_a INT, arg_b INT,\n"
    "  recorded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,\n"
    "  KEY idx_event (event_id)\n"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n";

/* Drops in reverse dependency order. */
const char *DB_DROP_SQL =
    "SET FOREIGN_KEY_CHECKS = 0;\n"
    "DROP TABLE IF EXISTS Telemetry;\n"
    "DROP TABLE IF EXISTS RunScores;\n"
    "DROP TABLE IF EXISTS Players;\n"
    "DROP TABLE IF EXISTS Ships;\n"
    "DROP TABLE IF EXISTS Planets;\n"
    "DROP TABLE IF EXISTS StarSystems;\n"
    "DROP TABLE IF EXISTS Factions;\n"
    "DROP TABLE IF EXISTS Encounters;\n"
    "DROP TABLE IF EXISTS LootEntries;\n"
    "DROP TABLE IF EXISTS LootTables;\n"
    "DROP TABLE IF EXISTS GearSets;\n"
    "DROP TABLE IF EXISTS ItemStats;\n"
    "DROP TABLE IF EXISTS Items;\n"
    "DROP TABLE IF EXISTS BossPhases;\n"
    "DROP TABLE IF EXISTS Bosses;\n"
    "DROP TABLE IF EXISTS Affixes;\n"
    "DROP TABLE IF EXISTS EnemyArchetypes;\n"
    "DROP TABLE IF EXISTS Effects;\n"
    "DROP TABLE IF EXISTS StatCaps;\n"
    "DROP TABLE IF EXISTS CoreAttributes;\n"
    "SET FOREIGN_KEY_CHECKS = 1;\n";

const char *DB_TABLE_NAMES[] = {
    "CoreAttributes",
    "StatCaps",
    "Effects",
    "EnemyArchetypes",
    "Affixes",
    "Bosses",
    "BossPhases",
    "Items",
    "ItemStats",
    "GearSets",
    "LootTables",
    "LootEntries",
    "Encounters",
    "Factions",
    "StarSystems",
    "Planets",
    "Ships",
    "Players",
    "RunScores",
    "Telemetry"};

int dbTableCount()
{
    return (int)(sizeof(DB_TABLE_NAMES) / sizeof(DB_TABLE_NAMES[0]));
}

const char *dbTableName(int index)
{
    if (index < 0 || index >= dbTableCount())
        return "";

    return DB_TABLE_NAMES[index];
}
