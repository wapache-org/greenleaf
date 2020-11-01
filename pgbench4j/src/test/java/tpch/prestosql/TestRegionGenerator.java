/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package tpch.prestosql;

import org.junit.Test;
import tpch.prestosql.RegionGenerator;

import static tpch.GeneratorAssertions.assertEntityLinesMD5;

public class TestRegionGenerator
{
    @SuppressWarnings("SpellCheckingInspection")
    @Test
    public void testGenerator()
    {
        assertEntityLinesMD5(new RegionGenerator(), "c235841b00d29ad4f817771fcc851207");
    }
}
