/*********************************************************************
 *  Author  : Tino Weinkauf
 *  Init    : Thursday, February 01, 2018 - 20:35:30
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/dh2320lab2/chaikin.h>
#include <cstdlib>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Chaikin::processorInfo_
{
    "org.inviwo.Chaikin",      // Class identifier
    "Chaikin",                // Display name
    "DH2320",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo Chaikin::getProcessorInfo() const
{
    return processorInfo_;
}


Chaikin::Chaikin()
    :Processor()
    ,portInLines("InLines")
    ,portOutLines("OutLines")
    ,propMinNumDesiredPoints("MinNumDesiredPoints", "Num Points", 100, 1, 200, 1)
{
    addPort(portInLines);
    addPort(portOutLines);
    addProperty(propMinNumDesiredPoints);
}

/*  Applies Chaikin's Corner Cutting algorithm.

    ControlPolygon is an array of points defining the line segments to be subdivided.
    MinNumDesiredPoints is the minimum number of points the resulting curve should contain.
    Curve is the array of points of the resulting curve after applying corner cutting.
*/
void Chaikin::CornerCutting(const std::vector<vec3>& ControlPolygon,
                            const size_t MinNumDesiredPoints,
                            std::vector<vec3>& Curve)
{
    //TODO: Extend and edit this code


    using std::cout;
    using std::cerr;
    using std::endl;
    // cerr <<"ny polygon. size ";
    // cerr << ControlPolygon.size() <<endl;
    // cerr <<"MinNumDesiredPoints ";
    // cerr << MinNumDesiredPoints <<endl;
    // cerr << "NumPointsPerPolygonLeg nedan ";
    // cerr << NumPointsPerPolygonLeg <<endl;
    // cerr << "--" <<endl;


    const size_t NumPointsPerPolygonLeg = MinNumDesiredPoints / ControlPolygon.size();
    size_t target = NumPointsPerPolygonLeg / 2; //number of iters



    //size_t target = (MinNumDesiredPoints-1) / ControlPolygon.size() ;
    if (target == 0){
      target ++;
    }
    Curve.clear();
    Curve.reserve(NumPointsPerPolygonLeg * ControlPolygon.size());
    // cerr << "NumPointsPerPolygonLeg " << NumPointsPerPolygonLeg +1 <<endl;
    // cerr << "target " << target <<endl;

    std::vector<vec3> input = ControlPolygon;
    std::vector<vec3> output; // our output
    output.reserve(2*input.size());

    size_t k = 0;
    float t = 0.25;
    //size_t size = input.size();

    size_t bonus =0;
    std::vector<size_t> binVector; // binary vector, true if keep point
    std::vector<vec3> compare; //to compare
    compare = ControlPolygon;


    while ((MinNumDesiredPoints > output.capacity()) || k==0){
      bonus++;

      size_t size = input.size();
      output.clear();
      output.reserve(2*size);

      for(size_t i(0);i<input.size();i++){
        const vec3& LeftPoint = input[i];
        const vec3& RightPoint = input[(i+1) % input.size()];
        t = 0.25;
        output.push_back((1-t) * LeftPoint + t * RightPoint);
        t = 0.75;
        output.push_back((1-t) * LeftPoint + t * RightPoint);
      }
      input=output;
      k++;


      // BONUS BELOW
      // if (bonus % 2 == 0){
      //   //now compare
      //   binVector.clear();
      //   binVector.reserve(output.size());
      //   for (size_t t(0); t<output.size(); t++){
      //     binVector.push_back(1);
      //   }
      //   for(size_t j(0);j<compare.size();j++){
      //     const vec3& LeftPoint = compare[j];
      //     const vec3& RightPoint = compare[(j+1) % compare.size()];
      //
      //     size_t start = 4*j;
      //     size_t end = 4*(j+1)-1;
      //
      //     float d = (float) bonus;
      //
      //     const vec3& LeftPointOutput = output[start];
      //     const vec3& RightPointOutput = output[end % output.size()];
      //
      //
      //     //now see if they are parallell, if so, remove the points
      //     float a0 = LeftPoint[0];
      //     float a1 = LeftPoint[1];
      //
      //     float b0 = RightPoint[0];
      //     float b1 = RightPoint[1];
      //
      //     float c0 = LeftPointOutput[0];
      //     float c1 = LeftPointOutput[1];
      //
      //     float d0 = RightPointOutput[0];
      //     float d1 = RightPointOutput[1];
      //
      //
      //     if ( abs((b0-a0)*(d1-c1)-(b1-a1)*(d0-c0)) < 0.01 ){ //0.01 4life
      //       //cerr << "prod " <<(b0-a0)*(d1-c1)-(b1-a1)*(d0-c0) <<endl;
      //       for(size_t k(start);k< end;k++){
      //         binVector[k] = 0;
      //       }
      //     }
      //   }
      //   input=ControlPolygon;
      //   input.clear();
      //
      //   for(size_t l(0);l<binVector.size();l++){
      //     if (binVector[l] == 1){
      //       input.push_back(output[l]);
      //     }
      //   }
      //   compare=output;
      //   output=input;
      // }

    }

    Curve = output;

}

void Chaikin::process()
{
    //Get the input data
    auto MultiInLines = portInLines.getVectorData();

    //Prepare output data
    auto OutLines = std::make_shared<Mesh>(DrawType::Lines, ConnectivityType::Strip);
    auto OutVertexBuffer = std::make_shared<Buffer<vec3> >();
    auto OutVertices = OutVertexBuffer->getEditableRAMRepresentation();
    OutLines->addBuffer(BufferType::PositionAttrib, OutVertexBuffer);

    for(auto InLines : MultiInLines)
    {
        //Vertex data
        auto pit = util::find_if(InLines->getBuffers(), [](const auto& buf)
        {
            return buf.first.type == BufferType::PositionAttrib;
        });
        if (pit == InLines->getBuffers().end()) return; //could not find a position buffer
        // - in RAM
        const auto posRam = pit->second->getRepresentation<BufferRAM>();
        if (!posRam) return; //could not find a position buffer ram
        // - 3D
        if (posRam->getDataFormat()->getComponents() != 3) return; //Only 3 dimensional meshes are supported
        // - save into a reasonable format with transformed vertices
        std::vector<glm::vec3> AllVertices;
        Matrix<4, float> Trafo = InLines->getWorldMatrix();
        const size_t NumInVertices = posRam->getSize();
        AllVertices.reserve(NumInVertices);
        for(size_t i(0);i<NumInVertices;i++)
        {
            dvec3 Position = posRam->getAsDVec3(i);
            glm::vec4 HomogeneousPos(Position.x, Position.y, Position.z, 1.0f);
            glm::vec4 TransformedHomogeneousPos = Trafo * HomogeneousPos;

            AllVertices.push_back( vec3(TransformedHomogeneousPos.x / TransformedHomogeneousPos.w,
                                        TransformedHomogeneousPos.y / TransformedHomogeneousPos.w,
                                        TransformedHomogeneousPos.z / TransformedHomogeneousPos.w
                                        ) );
        }

        //For each line buffer
        const auto& AllIndexBuffers = InLines->getIndexBuffers();
        for(const auto& IdxBuffer : AllIndexBuffers)
        {
            //Well, do we actually have lines? If not, next buffer!
            if (IdxBuffer.first.dt != DrawType::Lines) continue;

            //Get the indices of the lines
            const auto& Indices = IdxBuffer.second->getRAMRepresentation()->getDataContainer();

            //Create a simple vector of line vertices for the corner cutting
            std::vector<glm::vec3> LineVertices;
            LineVertices.reserve(Indices.size());
            for(const auto& idx : Indices)
            {
                if (LineVertices.empty() || LineVertices.back() != AllVertices[idx])
                {
                    LineVertices.push_back(AllVertices[idx]);
                }
            }

            //Remove a possibly duplicated first/last point (closed loop case),
            // since we assume a closed loop anyway.
            if (LineVertices.size() > 1 && LineVertices[0] == LineVertices[LineVertices.size() - 1])
            {
                LineVertices.pop_back();
            }

            //Cut the corners!
            std::vector<vec3> ChaikinVertices;
            CornerCutting(LineVertices, propMinNumDesiredPoints.get(), ChaikinVertices);
            const size_t NumNewVertices = ChaikinVertices.size();

            //Write out
            auto OutIndexBuffer = std::make_shared<IndexBuffer>();
            auto OutIndices = OutIndexBuffer->getEditableRAMRepresentation();
            auto OutIndexBufferPoints = std::make_shared<IndexBuffer>();
            auto OutIndicesPoints = OutIndexBufferPoints->getEditableRAMRepresentation();
            OutLines->addIndicies(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::Strip), OutIndexBuffer);
            OutLines->addIndicies(Mesh::MeshInfo(DrawType::Points, ConnectivityType::None), OutIndexBufferPoints);
            const size_t PreviousNumVertices = OutVertices->getSize();
            OutVertices->reserve(NumNewVertices + PreviousNumVertices);
            for(size_t i(0);i<NumNewVertices;i++)
            {
                OutVertices->add(ChaikinVertices[i]);
                OutIndices->add((uint32_t)(PreviousNumVertices + i));
                OutIndicesPoints->add((uint32_t)(PreviousNumVertices + i));
            }
            OutIndices->add((uint32_t)(PreviousNumVertices)); //Close loop.
        }
    }

    //Push it out!
    portOutLines.setData(OutLines);
}

} // namespace
} // namespace
